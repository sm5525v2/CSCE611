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
    num_regions = 0;
    gauge = 0;
    Console::puts("Constructed VMPool object.\n");
}

unsigned long VMPool::allocate(unsigned long _size) {
    unsigned long num_pages = (_size / ContFramePool::FRAME_SIZE) + (_size % ContFramePool::FRAME_SIZE == 0 ? 0 : 1);
    if(num_regions == 0 && _size <= size) {
	regions[0].start_address = base_address + ContFramePool::FRAME_SIZE;
	regions[0].size = num_pages * ContFramePool::FRAME_SIZE;
	gauge = regions[0].size;
	num_regions++;
	Console::puts("Allocated region of memory.\n");
	return regions[num_regions-1].start_address;
    } else if(num_regions < 256 && _size <= size - gauge){
	regions[num_regions].start_address = regions[num_regions - 1].start_address + regions[num_regions -1].size;
	regions[num_regions].size = num_pages * ContFramePool::FRAME_SIZE;
	gauge += regions[num_regions].size;
        num_regions++;
        Console::puts("Allocated region of memory.\n");
        return regions[num_regions-1].start_address;
    } else {
	return 0;
    }
}

void VMPool::release(unsigned long _start_address) {
    unsigned long release_idx = 99999;

    for(unsigned int i = 0; i < num_regions; i++) {
    	if(regions[i].start_address == _start_address){
	    release_idx = i;
	    break;
	}
    }
    unsigned long num_pages = (regions[release_idx].size / ContFramePool::FRAME_SIZE);
    unsigned long page_no = _start_address / ContFramePool::FRAME_SIZE;
    for(unsigned int i = 0; i < num_pages; i++) {
    	page_table->free_page(page_no);
	page_no++;
    }

    //pull region to previous idx
    for(unsigned int i = release_idx; i < num_regions - 1; i++) {
	regions[i] = regions[i + 1];
    }


    gauge -= regions[release_idx].size;
    num_regions--;

    Console::puts("Released region of memory.\n");
}

bool VMPool::is_legitimate(unsigned long _address) {
    for(unsigned int i = 0; i < num_regions; i++) {
    	if(regions[i].start_address <= _address && regions[i].start_address + regions[i].size >= _address){
	    Console::puts("Checked whether address is part of an allocated region.\n");
	    Console::puts("legitimate\n");
	    return true;
	}
    }
    Console::puts("Checked whether address is part of an allocated region.\n");
    Console::puts("not legitimate\n");
    return false;
}

