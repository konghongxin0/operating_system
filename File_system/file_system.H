/* 
    File: file_system.H

    Author: R. Bettati
            Department of Computer Science
            Texas A&M University
    Date  : 10/04/05

    Description: Simple File System.
    

*/

#ifndef _FILE_SYSTEM_H_                   // include file only once
#define _FILE_SYSTEM_H_

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/
#define SYSTEM_DISK_SIZE2 10485760
#define BLOCKSIZE 512
#define SYSTEM_BLOCKS SYSTEM_DISK_SIZE2/BLOCKSIZE

#define HEADER_SIZE 12

#define FREE    0x0000
#define USED    0xFFFF
#define BLOCK_DATA_SIZE 125

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/
#include "utils.H"
#include "file.H"
#include "simple_disk.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */ 
/*--------------------------------------------------------------------------*/
static unsigned char disk_buff[BLOCKSIZE];//for I/O

    struct BLOCK{
        unsigned int id;
        unsigned int availability;
        unsigned int size;
        unsigned int data[125];
    };

    static BLOCK* block=(BLOCK*)disk_buff;

/*--------------------------------------------------------------------------*/
/* FORWARD DECLARATIONS */ 
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* F i l e S y s t e m  */
/*--------------------------------------------------------------------------*/
class File;

class FileSystem {

friend class File; /* -- not sure if we need this; feel free to delete */

private:
     /* -- DEFINE YOUR FILE SYSTEM DATA STRUCTURES HERE. */
     
    SimpleDisk * disk;
     unsigned int size; //size of disk
     File*  files;
     unsigned int num_files;//number of files
     unsigned int block_num;//current block to look for free blocks

public:
   void setdisk(SimpleDisk* _disk){
		disk = _disk;
	};
    FileSystem();
    /* Just initializes local data structures. Does not connect to disk yet. */
    
    bool Mount(SimpleDisk * _disk);
    /* Associates this file system with a disk. Limit to at most one file system per disk.
     Returns true if operation successful (i.e. there is indeed a file system on the disk.) */
    
    static bool Format(SimpleDisk * _disk, unsigned int _size);
    /* Wipes any file system from the disk and installs an empty file system of given size. */
    
    File * LookupFile(int _file_id);
    /* Find file with given id in file system. If found, return the initialized
     file object. Otherwise, return null. */

    bool LookupFile(unsigned int _file_id, File * _file);
    
    bool CreateFile(int _file_id);
    /* Create file with given id in the file system. If file exists already,
     abort and return false. Otherwise, return true. */
    
    bool DeleteFile(int _file_id);
    /* Delete file with given id in the file system; free any disk block occupied by the file. */
   void push_back_file(File* file);
   bool remove_file( unsigned int _file_id);
   unsigned int AllocateBlock(unsigned int desired);
   /*Allocates a block from free list, for use in file*/
   void DeallocateBlock(unsigned int);
   /*Deallocates a block from free list, for use in file*/

};
#endif