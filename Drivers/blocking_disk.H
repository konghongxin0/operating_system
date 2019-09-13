/*
     File        : blocking_disk.H

     Author      : Hongxin Kong

     Date        : 
     Description : 

*/

#ifndef _BLOCKING_DISK_H_
#define _BLOCKING_DISK_H_

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "simple_disk.H"
#include "scheduler.H"
#include "thread.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */ 
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* B l o c k i n g D i s k  */
/*--------------------------------------------------------------------------*/
class Scheduler;
class Queue;
class BlockingDisk : public SimpleDisk {
protected:
  Scheduler *scheduler;
  Queue* blocked_thread_queue_head;
     void wait_until_ready();
     void add_to_queue(Thread *_thread);
int size;
public:
   BlockingDisk(DISK_ID _disk_id, unsigned int _size,Scheduler *scheduler);
   /* Creates a BlockingDisk device with the given size connected to the 
      MASTER or SLAVE slot of the primary ATA controller.
      NOTE: We are passing the _size argument out of laziness. 
      In a real system, we would infer this information from the 
      disk controller. */
  void resume_from_queue();
     /* Resume a thread from the blocked thread queue when the disk has completed the
        read/write. */
     
   bool is_ready();
   /* DISK OPERATIONS */


};
/*
class MirroredDisk:public BlockingDisk{
public:
    MirroredDisk(DISK_ID _disk_id, unsigned int _size): SimpleDisk(_disk_id, _size){};
    void read(unsigned long _block_no,unsigned char* _buf);
    void write(unsigned long _block_no, unsigned char* _buf);
    Thread* get_thread();
private:
    queue* blockq;
};*/
#endif
