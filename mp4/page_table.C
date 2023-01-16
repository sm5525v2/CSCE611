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
   //setting up page directory
   page_directory = (unsigned long *) (process_mem_pool->get_frames(1) * PAGE_SIZE); 

   //setting up page table
   unsigned long* page_table = (unsigned long *) (process_mem_pool->get_frames(1) * PAGE_SIZE); 

   unsigned long address=0;
   unsigned int i;

   // map the first 4MB of memory
   for(i=0; i<ENTRIES_PER_PAGE; i++) {
      page_table[i] = address | 3; // attribute set to: supervisor level, read/write, present(011 in binary)
      address = address + PAGE_SIZE; // 4096 = 4kb
   };

   //filling in the page directory entries
   //fill the first entry of the page directory
   page_directory[0] = (unsigned long) page_table; // attribute set to: supervisor level, read/write, present(011 in binary)
   page_directory[0] = page_directory[0] | 3;

   for(i=1; i<ENTRIES_PER_PAGE - 1; i++){
      page_directory[i] = 0 | 2; // attribute set to: supervisor level, read/write, not present(010 in binary)
   };

   //make recursive
   page_directory[ENTRIES_PER_PAGE - 1] = (unsigned long) page_directory | 3;

   //initialize VMPIdx
   VMPIdx = 0;

   Console::puts("Constructed Page Table object\n");
}

void PageTable::load()
{
   current_page_table = this;
   write_cr3((unsigned long) page_directory); // put that page directory address into CR3
   Console::puts("Loaded page table\n");
}

void PageTable::enable_paging()
{
   write_cr0(read_cr0() | 0x80000000); // set the paging bit in CR0 to 1
   PageTable::paging_enabled = 1;
   Console::puts("Enabled paging\n");
}

void PageTable::handle_fault(REGS * _r)
{
   unsigned long address = read_cr2();

   bool isLegit = false;
   for(int i = 0; i < current_page_table->VMPIdx; i++) {
   	if(current_page_table->VMPList[i]->is_legitimate(address)){
	    isLegit = true;
	    break;
	}
   }
   if(!isLegit) {
       Console::puts("not legitimate\n");
   }

   unsigned long* pde = PDE_address(address);   

   unsigned long* pte = PTE_address(address);

   //should check whether need to add a page table or just add a page
   //if address is first address of new page table, need to add page table
   //else, just add page

   if(*pde & 1 != 1) { //not present
      //new page table
      *pde = (process_mem_pool->get_frames(1) * PAGE_SIZE);
      *pde |= 3;
   }
   *pte = (process_mem_pool->get_frames(1) * PAGE_SIZE);
   *pte |= 3;

   Console::puts("handled page fault\n");
}

void PageTable::register_pool(VMPool * _vm_pool) {
     VMPList[VMPIdx] = _vm_pool;
     VMPIdx++;
     Console::puts("registered VM pool\n");
 }

 void PageTable::free_page(unsigned long _page_no) {
     
     unsigned long table_index = _page_no >> 10;
     unsigned long* page_table = (unsigned long*) (0xFFC00000 | (table_index << 12));
     unsigned long page_index = (_page_no >> 10) & 0x3FF;
     
     if(page_table[page_index] & 1 == 1) {
	process_mem_pool->release_frames(_page_no);
	page_directory[table_index] |= 2;
	write_cr3(read_cr3());
	Console::puts("released frames\n");
     }

     Console::puts("freed page\n");
 }

unsigned long * PageTable::PDE_address(unsigned long addr) {
    unsigned long pde = addr >> 22;
    pde = pde << 2;
    pde |= 0xFFFFF000;
    return (unsigned long*) pde;

}

unsigned long * PageTable::PTE_address(unsigned long addr) {
    unsigned long pte = addr >> 12;
    pte = pte << 2;
    pte |= 0xFFC00000;
    return (unsigned long*) pte;
}
