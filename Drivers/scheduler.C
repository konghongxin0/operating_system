/*
 File: scheduler.C
 
 Author:Hongxin Kong
 Date  :04/10/2019
 
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

#ifndef NULL
#define NULL 0L
#endif


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

/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   S c h e d u l e r  */
/*--------------------------------------------------------------------------*/


Scheduler::Scheduler() {
  size=0;
  //ready_queue = new Queue(); Already construct
 	this ->disk =NULL;
   Console::puts("Constructed Scheduler.\n");
}

void Scheduler::yield() {
  if(disk !=NULL && disk->is_ready())
	disk->resume_from_queue();

    if (size!=0){
            --size;
            Thread* curr= ready_queue.dequeue(); //get next queue waiting
            Thread::dispatch_to(curr);// run new thread
        }
    else{
      Console::puts("CREATING THREAD 0...\n");
    }
}

void Scheduler::resume(Thread * _thread) {
	ready_queue.enqueue(_thread);//add thread to queue
        ++size;
}

void Scheduler::add(Thread * _thread) {
  	ready_queue.enqueue(_thread);//add thread to queue
        ++size;
}

void Scheduler::terminate(Thread * _thread) {
  Console::puts("IN terminate of the schedler.C of ");
  Console::puti(_thread->ThreadId());
  Console::puts("\n");

  	 bool found=false;
        for (int i=0;i<size;++i){
            Thread* temp=ready_queue.dequeue();
            if (temp->ThreadId()==_thread->ThreadId())
                found=true;
            else 
                ready_queue.enqueue(temp);
        }
        if (found)
            --size;
}

void Scheduler::add_BlockDisk(BlockingDisk * _disk) {
	disk = _disk;
}