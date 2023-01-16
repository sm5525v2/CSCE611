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
   page_directory = (unsigned long *) (kernel_mem_pool->get_frames(1) * PAGE_SIZE); 

   //setting up page table
   unsigned long* page_table = (unsigned long *) (kernel_mem_pool->get_frames(1) * PAGE_SIZE); 

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

   for(i=1; i<ENTRIES_PER_PAGE; i++){
      page_directory[i] = 0 | 2; // attribute set to: supervisor level, read/write, not present(010 in binary)
   };
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
   unsigned long table_index = address >> 22; //right 10 bit is table index

   unsigned long* pg_directory = (unsigned long*) read_cr3();
   unsigned long* page_table;
   //should check whether need to add a page table or just add a page
   //if address is first address of new page table, need to add page table
   //else, just add page

   //check directory entry to check page table present or not
   if(pg_directory[table_index] & 1 == 1) { //present
      page_table = (unsigned long*) (pg_directory[table_index] & 0xFFFFF000);
   } else { //not present
      //new page table
      page_table = (unsigned long*) (PAGE_SIZE * kernel_mem_pool->get_frames(1)); 
      pg_directory[table_index] = (unsigned long) page_table;
      pg_directory[table_index] = pg_directory[table_index] | 3;

      unsigned int i;
      for(i=0; i<ENTRIES_PER_PAGE; i++) {
         page_table[i] = 2;
      };
      page_table = (unsigned long*) ((unsigned long)page_table & 0xFFFFF000);
   }
   unsigned long page_index = address & 0x3FF000;
   page_index = page_index >> 12;

   page_table[page_index] = (process_mem_pool->get_frames(1) * PAGE_SIZE); 
   page_table[page_index] = page_table[page_index] | 3;

   Console::puts("handled page fault\n");
}

