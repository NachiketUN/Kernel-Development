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
  if(ready_queue.head_list == nullptr) return;
  if(Machine::interrupts_enabled()) Machine::disable_interrupts();
  Thread *thread = ready_queue.get_front_thread();
  Thread::dispatch_to(thread);
  Machine::enable_interrupts();
}

void Scheduler::resume(Thread * _thread) {
    if(Machine::interrupts_enabled()) Machine::disable_interrupts();

    ready_queue.add_thread_node(_thread);
    Machine::enable_interrupts();

}

void Scheduler::add(Thread * _thread) {
    if(Machine::interrupts_enabled()) Machine::disable_interrupts();

    ready_queue.add_thread_node(_thread);
    Machine::enable_interrupts();

}

void Scheduler::terminate(Thread * _thread) {
    if(Machine::interrupts_enabled()) Machine::disable_interrupts();

    ready_queue.delete_thread_node(_thread);
}
