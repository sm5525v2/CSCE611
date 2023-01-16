/*
     File        : blocking_disk.c

     Author      : 
     Modified    : 

     Description : 
i
*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "utils.H"
#include "console.H"
#include "blocking_disk.H"
#include "scheduler.H"

extern Scheduler* SYSTEM_SCHEDULER;

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

BlockingDisk::BlockingDisk(DISK_ID _disk_id, unsigned int _size) 
  : SimpleDisk(_disk_id, _size) {
	  diskQueue = new Queue();
}

/*--------------------------------------------------------------------------*/
/* SIMPLE_DISK FUNCTIONS */
/*--------------------------------------------------------------------------*/

void BlockingDisk::read(unsigned long _block_no, unsigned char * _buf) {
  SimpleDisk::read(_block_no, _buf);
  SYSTEM_SCHEDULER->setDiskInUse(false);
}


void BlockingDisk::write(unsigned long _block_no, unsigned char * _buf) {
  SimpleDisk::write(_block_no, _buf);
  SYSTEM_SCHEDULER->setDiskInUse(false);
}

bool BlockingDisk::is_ready() {
	return SimpleDisk::is_ready();
}

void BlockingDisk::wait_until_ready() {
	if(!is_ready()) {
		Thread* thread = Thread::CurrentThread();
		if(!SYSTEM_SCHEDULER->getDiskInUse()) {//if disk is not in use, push to disk block queue to use later
			pushToQueue(thread);
			SYSTEM_SCHEDULER->setDiskInUse(true);
		} else {//if disk is not ready and disk already in use, push to normal ready queue to call later
			SYSTEM_SCHEDULER->resume(thread);
		}
		SYSTEM_SCHEDULER->yield();
	}
}

void BlockingDisk::pushToQueue(Thread* thread){
	diskQueue->push(thread);
}
