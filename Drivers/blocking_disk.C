/*
     File        : blocking_disk.c

     Author      : Hongxin Kong
     Modified    : 

     Description : 

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
#include "simple_disk.H"
#include"scheduler.H"
#include"thread.H"


extern Scheduler* SYSTEM_SCHEDULER;
/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

BlockingDisk::BlockingDisk(DISK_ID _disk_id, unsigned int _size, Scheduler *_scheduler) 
  : SimpleDisk(_disk_id, _size) {
    this->scheduler = _scheduler;
    size = 0;
    this->blocked_thread_queue_head = new Queue();
}
/*
void BlockingDisk::wait_until_ready(){
	Console::puti(is_ready());
	Console::puts("in blocking disk \n");
       if(!is_ready()) {
        Thread *th = Thread::CurrentThread();
        this->add_to_queue(th);
        this->scheduler->yield();
      }
  }
  */
  void BlockingDisk::wait_until_ready(){
   while (!is_ready()) {
            SYSTEM_SCHEDULER->resume(Thread::CurrentThread());//place current thread on ready queue
            SYSTEM_SCHEDULER->yield();//yield processor while we wait for io to be ready
            }
  }

  void BlockingDisk::add_to_queue(Thread *_thread) {
    blocked_thread_queue_head->enqueue(_thread);//add thread to queue
    ++size;
}
void BlockingDisk::resume_from_queue() {
  if(size == 0) {
    Console::puts("Error: Blocked queue is NULL.\n");
  } 
  else {
    Thread *th = this->blocked_thread_queue_head->dequeue();
    this->scheduler->resume(th); 
  }
}
bool BlockingDisk::is_ready() {
  return SimpleDisk::is_ready();
}




/*--------------------------------------------------------------------------*/
/* SIMPLE_DISK FUNCTIONS */
/*--------------------------------------------------------------------------*/
/*
void BlockingDisk::read(unsigned long _block_no, unsigned char * _buf) {
  // -- REPLACE THIS!!!
  SimpleDisk::read(_block_no, _buf);
}
void BlockingDisk::write(unsigned long _block_no, unsigned char * _buf) {
  // -- REPLACE THIS!!!
  SimpleDisk::read(_block_no, _buf);
}
*/