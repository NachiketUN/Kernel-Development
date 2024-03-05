#include "assert.H"
#include "exceptions.H"
#include "console.H"
#include "paging_low.H"
#include "page_table.H"

PageTable * PageTable::current_page_table = NULL;
unsigned int PageTable::paging_enabled = 0;
ContFramePool * PageTable::kernel_mem_pool = NULL;
ContFramePool * PageTable::process_mem_pool = NULL;
unsigned long PageTable::shared_size = 0;
VMPool *PageTable::vm[];



void PageTable::init_paging(ContFramePool * _kernel_mem_pool,
                            ContFramePool * _process_mem_pool,
                            const unsigned long _shared_size)
{
    kernel_mem_pool = _kernel_mem_pool;
    process_mem_pool = _process_mem_pool;
    shared_size = _shared_size;
    Console::puts("Initialized Paging System\n");
}

PageTable::PageTable()
{
    page_directory = (unsigned long *) (kernel_mem_pool->get_frames(1) * PAGE_SIZE);
    auto* page_table = (unsigned long *) (kernel_mem_pool->get_frames(1) * PAGE_SIZE);

    unsigned long address = 0;
    for(int i=0; i<1024; i++){
        page_table[i] = address | 3;
        address += PAGE_SIZE;
    }

    page_directory[0] = (unsigned long) page_table | 3;
    for(int i=1; i<1024; i++)
    {
        if(i == 1023)
            page_directory[i] = (unsigned long) page_directory | 3;
        else
            page_directory[i] = 2; // attribute set to: supervisor level, read/write, not present(010 in binary)
    };

    Console::puts("Constructed Page Table object\n");
}


void PageTable::load()
{
    current_page_table = this;
    write_cr3((unsigned long) page_directory);
    Console::puts("Loaded page table\n");
}

void PageTable::enable_paging()
{
    paging_enabled = true;
    write_cr0(read_cr0() | 0x80000000);
    Console::puts("Enabled paging\n");
}

unsigned long * PageTable::PDE_address(unsigned long addr){
    unsigned long pde_bits = (addr >> 20);
    pde_bits |= 0xFFFFF000;
    pde_bits &= 0xFFFFFFFC;
    return (unsigned long*) pde_bits;
}

unsigned long * PageTable::PTE_address(unsigned long addr){
    unsigned long pte_bits = (addr >> 10);
    pte_bits |= 0xFFC00000;
    pte_bits &= 0xFFFFFFFC;
    return (unsigned long*) (pte_bits);

}

void PageTable::handle_fault(REGS * _r)
{
    Console::puts("ENTERED handle_fault\n");
    unsigned int error = _r->err_code;
    if(error & 1){
        Console::puts("Protection Fault, NOT a page fault");
        return;
    }
    unsigned long page_fault_address = read_cr2();

    bool valid_pool_page = false;
    int num_of_valid_pools = 0;

    for(int i=0; i<512; i++){
        if(vm[i] != nullptr){
            if(vm[i]->is_legitimate(page_fault_address)){
                valid_pool_page=true;
                break;
            }
            else{
                num_of_valid_pools++;
            }
        }
    }
    if(!valid_pool_page && num_of_valid_pools){
        Console::puts("No valid pool associated to address\n");
        assert(false);
    }

    unsigned long *pde_pointer = PDE_address(page_fault_address);
    unsigned long *pte_pointer = PTE_address(page_fault_address);

    if(*pde_pointer & 1){
        *pte_pointer = (unsigned long) (process_mem_pool->get_frames(1) * PAGE_SIZE) | 3;
    }
    else{
        *pde_pointer = (unsigned long) (process_mem_pool->get_frames(1) * PAGE_SIZE) | 3;

        *pte_pointer = (unsigned long) (process_mem_pool->get_frames(1) * PAGE_SIZE) | 3;

    }
//    bool page_table_newly_allocated = false;
//    //If page directory entry(page table) NOT valid, allocate a frame and make it valid
//    if(!(page_directory_address[directory_entry_index] & 1)){
//        page_directory_address[directory_entry_index] = (unsigned long) (kernel_mem_pool->get_frames(1) * PAGE_SIZE) | 3;
//        page_table_newly_allocated = true;
//    }
//    page_table = (unsigned long*) (page_directory_address[directory_entry_index] & (page_mask + directory_mask));
//
//    if(page_table_newly_allocated){
//        for(int i=0; i<1024; i++){
//            //set last three bits to 100 so that pages are not valid and are write protected since no page has been allocated yet
//            page_table[i] = 4;
//        }
//    }
//
//    // Get a frame for the actual page and store its address in page table
//    page_table[page_table_entry_index] = (unsigned long) (process_mem_pool->get_frames(1) * PAGE_SIZE) | 3;

    Console::puts("handled page fault\n");
}

void PageTable::register_pool(VMPool * _vm_pool)
{
    if(pool_number == 512){
        Console::puts("No free pools available");
        assert(false);
    }
    vm[pool_number] = _vm_pool;
    pool_number++;
    Console::puts("registered VM pool\n");
}

void PageTable::free_page(unsigned long _page_no) {
    unsigned long *pte = PTE_address(_page_no);
    if(*pte & 1){
        process_mem_pool->release_frames(*pte>>12);
    }
    *pte = 2;
    write_cr3((unsigned long) page_directory);
    Console::puts("freed page\n");
}
