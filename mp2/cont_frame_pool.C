/*
 File: ContFramePool.C
 
 Author:
 Date  : 
 
 */

/*--------------------------------------------------------------------------*/
/* 
 POSSIBLE IMPLEMENTATION
 -----------------------

 The class SimpleFramePool in file "simple_frame_pool.H/C" describes an
 incomplete vanilla implementation of a frame pool that allocates 
 *single* frames at a time. Because it does allocate one frame at a time, 
 it does not guarantee that a sequence of frames is allocated contiguously.
 This can cause problems.
 
 The class ContFramePool has the ability to allocate either single frames,
 or sequences of contiguous frames. This affects how we manage the
 free frames. In SimpleFramePool it is sufficient to maintain the free 
 frames.
 In ContFramePool we need to maintain free *sequences* of frames.
 
 This can be done in many ways, ranging from extensions to bitmaps to 
 free-lists of frames etc.
 
 IMPLEMENTATION:
 
 One simple way to manage sequences of free frames is to add a minor
 extension to the bitmap idea of SimpleFramePool: Instead of maintaining
 whether a frame is FREE or ALLOCATED, which requires one bit per frame, 
 we maintain whether the frame is FREE, or ALLOCATED, or HEAD-OF-SEQUENCE.
 The meaning of FREE is the same as in SimpleFramePool. 
 If a frame is marked as HEAD-OF-SEQUENCE, this means that it is allocated
 and that it is the first such frame in a sequence of frames. Allocated
 frames that are not first in a sequence are marked as ALLOCATED.
 
 NOTE: If we use this scheme to allocate only single frames, then all 
 frames are marked as either FREE or HEAD-OF-SEQUENCE.
 
 NOTE: In SimpleFramePool we needed only one bit to store the state of 
 each frame. Now we need two bits. In a first implementation you can choose
 to use one char per frame. This will allow you to check for a given status
 without having to do bit manipulations. Once you get this to work, 
 revisit the implementation and change it to using two bits. You will get 
 an efficiency penalty if you use one char (i.e., 8 bits) per frame when
 two bits do the trick.
 
 DETAILED IMPLEMENTATION:
 
 How can we use the HEAD-OF-SEQUENCE state to implement a contiguous
 allocator? Let's look a the individual functions:
 
 Constructor: Initialize all frames to FREE, except for any frames that you 
 need for the management of the frame pool, if any.
 
 get_frames(_n_frames): Traverse the "bitmap" of states and look for a 
 sequence of at least _n_frames entries that are FREE. If you find one, 
 mark the first one as HEAD-OF-SEQUENCE and the remaining _n_frames-1 as
 ALLOCATED.

 release_frames(_first_frame_no): Check whether the first frame is marked as
 HEAD-OF-SEQUENCE. If not, something went wrong. If it is, mark it as FREE.
 Traverse the subsequent frames until you reach one that is FREE or 
 HEAD-OF-SEQUENCE. Until then, mark the frames that you traverse as FREE.
 
 mark_inaccessible(_base_frame_no, _n_frames): This is no different than
 get_frames, without having to search for the free sequence. You tell the
 allocator exactly which frame to mark as HEAD-OF-SEQUENCE and how many
 frames after that to mark as ALLOCATED.
 
 needed_info_frames(_n_frames): This depends on how many bits you need 
 to store the state of each frame. If you use a char to represent the state
 of a frame, then you need one info frame for each FRAME_SIZE frames.
 
 A WORD ABOUT RELEASE_FRAMES():
 
 When we releae a frame, we only know its frame number. At the time
 of a frame's release, we don't know necessarily which pool it came
 from. Therefore, the function "release_frame" is static, i.e., 
 not associated with a particular frame pool.
 
 This problem is related to the lack of a so-called "placement delete" in
 C++. For a discussion of this see Stroustrup's FAQ:
 http://www.stroustrup.com/bs_faq2.html#placement-delete
 
 */
/*--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

#define K * 1024


/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "cont_frame_pool.H"
#include "console.H"
#include "utils.H"
#include "assert.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/
ContFramePool* ContFramePool::start_of_frame_pool_list;
ContFramePool* ContFramePool::end_of_frame_pool_list;
/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   C o n t F r a m e P o o l */
/*--------------------------------------------------------------------------*/

ContFramePool::ContFramePool(unsigned long _base_frame_no,
                             unsigned long _n_frames,
                             unsigned long _info_frame_no)
{
    // Bitmap must fit in a single frame!
    assert(_n_frames * 2 <= FRAME_SIZE * 8);
    base_frame_no = _base_frame_no;
    nframes = _n_frames;
    nFreeFrames = _n_frames;
    info_frame_no = _info_frame_no;

    // If _info_frame_no is zero then we keep management info in the first
    //frame, else we use the provided frame to keep management info
    if(info_frame_no == 0) {
        bitmap = (unsigned char *) (base_frame_no * FRAME_SIZE);
    } else {
        bitmap = (unsigned char *) (info_frame_no * FRAME_SIZE);
    }

    // Mark all the frames as free
    for(int frame_no = 0; frame_no < nframes; frame_no++){
        set_state(frame_no, FrameState::Free);
    }
    // Mark the first frame as being used if it is being used
    if(_info_frame_no == 0) {
        set_state(0, FrameState::HoS);
        nFreeFrames--;
    }
    // Initialise the pointers which manage the frame pool list
    if(ContFramePool::end_of_frame_pool_list == NULL){
        ContFramePool::start_of_frame_pool_list = this;
        ContFramePool::end_of_frame_pool_list = this;
    }
    else{
        ContFramePool::end_of_frame_pool_list->next = this;
        ContFramePool::end_of_frame_pool_list = this;
    }
    Console::puts("Frame Pool initialized\n");
}

void ContFramePool::print_bitmap(){
    Console::puts("Printing bitmap ===== \n");
    for(int j = 0; j < this->nframes; j++){
        char currentByte = bitmap[j];
        Console::puti(j);
        Console::puts(" ");
        for (int i = 7; i >= 0; i--) {
            // Check each bit in the byte and print it
            if (currentByte & (1 << i)) {
                Console::puts("1");
            } else {
                Console::puts("0");
            }
        }
        Console::puts("\n");
        if(j==2) break;
    }
}

unsigned long ContFramePool::get_frames(unsigned int _n_frames)
{
    assert(_n_frames <= this->nFreeFrames)
    unsigned long first_free_frame = -1;
    for(unsigned long frame_no = 0; frame_no < this->nframes; frame_no++){
        if(get_state(frame_no) == FrameState::Free){
            unsigned long right_frame_no = frame_no + 1;
            unsigned long no_free_frames = 1;
            while(right_frame_no < this->nframes && no_free_frames < _n_frames){
                if(get_state(right_frame_no) == FrameState::Free){
                    no_free_frames++;
                    right_frame_no++;
                }
                else
                    break;
            }
            if(no_free_frames == _n_frames){ // Found contiguous frames which are free
                first_free_frame = frame_no;
                break;
            }
        }
    }
    unsigned int frames_to_allocate = _n_frames;
    if(first_free_frame != -1){
        // Allocating the contiguous frames
        set_state(first_free_frame, FrameState::HoS); // Setting First Frame as Head of Sequence
        nFreeFrames--;
        frames_to_allocate--;
        unsigned long frame_no = first_free_frame + 1;
        while(frames_to_allocate>0){
            set_state(frame_no, FrameState::Used);
            frame_no++;
            frames_to_allocate--;
            nFreeFrames--;
        }

        return (first_free_frame + base_frame_no);
    }
    else
        return 0;
}

void ContFramePool::mark_inaccessible(unsigned long _base_frame_no,
                                      unsigned long _n_frames)
{
    for (int frame_no = _base_frame_no; frame_no < _base_frame_no + _n_frames; frame_no++){
        set_state(frame_no - this->base_frame_no, FrameState::Inaccessible);
    }
}

void ContFramePool::release_frames(unsigned long _first_frame_no)
{
    ContFramePool* frame_pool = ContFramePool::start_of_frame_pool_list;
    bool is_frame_pool_available = false;

    // Find frame pool which houses the _first_frame_no
    while(frame_pool != NULL){
        if(frame_pool->base_frame_no <= _first_frame_no && _first_frame_no < frame_pool->base_frame_no + frame_pool->nframes){
            is_frame_pool_available = true;
            break;
        }
        frame_pool = frame_pool->next;
    }

    if(is_frame_pool_available){
        unsigned long frame_index = _first_frame_no - frame_pool->base_frame_no;
        if(frame_pool->get_state(frame_index) != FrameState::HoS){
            Console::puts("First frame provided is NOT HoS, provide a correct first frame number");
        }
        else{
            // Free the HoS frame
            frame_pool->set_state(frame_index, FrameState::Free);
            frame_pool->nFreeFrames++;
            int frame_no = frame_index + 1;
            // Free rest of the frames
            while(frame_pool->get_state(frame_no) == FrameState::Used){
                frame_pool->set_state(frame_no,FrameState::Free);
                frame_no++;
                frame_pool->nFreeFrames++;
            }

        }
    }
    else{
        Console::puts("Frame NOT found in any of the frame pools");
    }

}

unsigned long ContFramePool::needed_info_frames(unsigned long _n_frames)
{
    return _n_frames / (16 K) + (_n_frames % (16 K) > 0 ? 1 : 0);
}

ContFramePool::FrameState ContFramePool::get_state(unsigned long _frame_no){
    unsigned int index = _frame_no / 4;
    unsigned int mask = 0x1 << ((_frame_no % 4) * 2);
    if((bitmap[index] & mask) == 0){
        mask <<= 1;
        if((bitmap[index] & mask) == 0)
            return ContFramePool::FrameState::Inaccessible;
        return ContFramePool::FrameState::Used;
    } else {
        mask <<= 1;
        if((bitmap[index] & mask) == 0)
            return ContFramePool::FrameState::HoS;
        return ContFramePool::FrameState::Free;
    }

}

void ContFramePool::set_state(unsigned long _frame_no, ContFramePool::FrameState _state) {
    unsigned int index = _frame_no / 4;
    unsigned int mask = 0x1 << ((_frame_no % 4) * 2);
    // 11 - Free
    // 10 - Used
    // 01 - HOS
    // 00 - Inaccessible
    switch(_state){
        case FrameState::Free:
            mask = 0x3 << ((_frame_no % 4)*2);
            bitmap[index] |= mask;
            break;
        case FrameState::Used:
            bitmap[index] ^= mask;
            break;
        case FrameState::HoS:
            mask <<= 1;
            bitmap[index] ^= mask;
            break;
        case FrameState::Inaccessible:
            mask = ~(0x3 << ((_frame_no % 4)*2));
            bitmap[index] &= mask;
            break;
    }
}

