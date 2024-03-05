/*
 File: vm_pool.C
 
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

#include "vm_pool.H"
#include "console.H"
#include "utils.H"
#include "assert.H"
#include "simple_keyboard.H"

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

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   V M P o o l */
/*--------------------------------------------------------------------------*/

VMPool::VMPool(unsigned long  _base_address,
               unsigned long  _size,
               ContFramePool *_frame_pool,
               PageTable     *_page_table) {
    base_address = _base_address;
    size = _size;
    frame_pool = _frame_pool;
    page_table = _page_table;
    page_table->register_pool(this);
//    Console::puts("Here after registering");
    memRegion = (mem_region*)(base_address);
    num_of_allocated_regions=1;

    memRegion[0].start_addr = base_address;
    memRegion[0].size = Machine::PAGE_SIZE;
//    Console::puts("something wrong?");

    for(int i=1; i<512; i++){
        memRegion[i].start_addr = 0;
        memRegion[i].size = 0;
    }

    Console::puts("Constructed VMPool object.\n");
}

unsigned long VMPool::allocate(unsigned long _size) {
    Console::puts("In allocate\n");
    if(num_of_allocated_regions>512){
        Console::puts("Ran out of regions to allocate\n");
        assert(false);
    }
    unsigned long num_of_pages = _size/Machine::PAGE_SIZE;
    unsigned long offset = _size%Machine::PAGE_SIZE;
    if(offset) num_of_pages++;

    unsigned long new_start_address = memRegion[num_of_allocated_regions-1].start_addr + memRegion[num_of_allocated_regions-1].size;

    memRegion[num_of_allocated_regions].start_addr = new_start_address;
    memRegion[num_of_allocated_regions].size = num_of_pages * Machine::PAGE_SIZE;
    num_of_allocated_regions++;

    Console::puts("Allocated region of memory.\n");
    return new_start_address;
}

void VMPool::release(unsigned long _start_address) {
    int mem_region_index=-1;
    for(int i=0;i<512;i++){
        if(memRegion[i].start_addr == _start_address){
            mem_region_index = i;
            break;
        }
    }
    if(mem_region_index == -1){
        Console::puts("No region found with given start address\n");
        assert(false);
    }
    unsigned long num_pages_in_region = memRegion[mem_region_index].size/Machine::PAGE_SIZE;
    unsigned long page_address = _start_address;
    for(int i = mem_region_index;i<num_of_allocated_regions-1;i++){
        memRegion[i] = memRegion[i+1];
    }
    memRegion[num_of_allocated_regions].start_addr = 0;
    memRegion[num_of_allocated_regions].size = 0;
    while(num_pages_in_region){
        page_table->free_page(page_address);
        page_address += Machine::PAGE_SIZE;
        num_pages_in_region--;
    }
    num_of_allocated_regions--;

    Console::puts("Released region of memory.\n");
}

bool VMPool::is_legitimate(unsigned long _address) {
    if(_address == base_address) return true;
    for(int i=0;i<num_of_allocated_regions;i++){
        if(_address >= memRegion[i].start_addr && _address <= (memRegion[i].start_addr + memRegion[i].size)){
            return true;
        }
    }
    Console::puts("Checked whether address is part of an allocated region.\n");

    return false;
}

