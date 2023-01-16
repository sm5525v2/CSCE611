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

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */
void enable_interrupts() {
	if(!Machine::interrupts_enabled()) Machine::enable_interrupts();
}

void disable_interrupts() {
        if(Machine::interrupts_enabled()) Machine::disable_interrupts();
}

/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   S c h e d u l e r  */
/*--------------------------------------------------------------------------*/

Scheduler::Scheduler() {
  readyQ = new Queue();
  Console::puts("Constructed Scheduler.\n");
}

void Scheduler::yield() {
	disable_interrupts();
	Thread* target = readyQ->pop();
	if(target) Thread::dispatch_to(target);
	Console::puts("Scheduler yeild called\n");
	enable_interrupts();
}

void Scheduler::resume(Thread * _thread) {
	disable_interrupts();
	readyQ->push(_thread);
	Console::puts("Scheduler resume called\n");
	enable_interrupts();
}

void Scheduler::add(Thread * _thread) {
	resume(_thread);
	Console::puts("Scheduler add called\n");
}

void Scheduler::terminate(Thread * _thread) {
	int n = readyQ->size();
	for(int i = 0; i < n; i++) {
		Thread* cur = readyQ->pop();
		if(_thread->ThreadId() != cur->ThreadId()) readyQ->push(cur);
	}
	Console::puts("Scheduler terminate called\n");
}
