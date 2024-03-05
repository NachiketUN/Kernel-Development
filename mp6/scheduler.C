/*
 File: scheduler.C
 
 Author:
 Date  :
 
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "scheduler.H"
#include "thread.H"
#include "console.H"
#include "utils.H"
#include "assert.H"
#include "simple_keyboard.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/
thread_node* thread_node::head_list;
thread_node* thread_node::tail_list;
/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   S c h e d u l e r  */
/*--------------------------------------------------------------------------*/

Scheduler::Scheduler() {
  Console::puts("Constructed Scheduler.\n");
}

void Scheduler::yield() {
    if(blockingDisk != nullptr && blockingDisk->is_ready() && blockingDisk->block_queue->empty()) {
        Thread *thread = blockingDisk->block_queue->get_front_thread();
        Thread::dispatch_to(thread);
    } else {
        if(ready_queue.head_list == nullptr) return;
        Thread *thread = ready_queue.get_front_thread();
        Thread::dispatch_to(thread);
    }

}

void Scheduler::resume(Thread * _thread) {
    ready_queue.add_thread_node(_thread);
}

void Scheduler::add(Thread * _thread) {

    ready_queue.add_thread_node(_thread);

}

void Scheduler::terminate(Thread * _thread) {

    ready_queue.delete_thread_node(_thread);
}

void Scheduler::add_disk(BlockingDisk* blockingDisk1) {
    blockingDisk = blockingDisk1;
}
