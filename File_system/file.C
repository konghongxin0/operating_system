/*
     File        : file.C

     Author      : Riccardo Bettati
     Modified    : 2017/05/01

     Description : Implementation of simple File class, with support for
                   sequential read/write operations.
*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "console.H"
#include "file.H"
#include "file_system.H"

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

File::File() {
    /* We will need some arguments for the constructor, maybe pointer to disk
     block with file management and allocation data. */
   file_id=0;
   file_size=0;
   cur_block=0;
   cur_position=0;
   block_nums=NULL;
    
}
File::File(unsigned int id) {
  file_id=id;
        if (FILE_SYSTEM->LookupFile(file_id, this))//this allows LookupFile to initialize file, returns false if not found
            Console::puts("Found File\n");
        else if (FILE_SYSTEM->CreateFile(file_id)){
            cur_block=0;//initialize empty file, if write occurs we will allocate memory later
            cur_position=0;
            file_size=0;
            block_nums=NULL;
        }
        else
            Console::puts("ERR cannot create file\n");
}
/*--------------------------------------------------------------------------*/
/* FILE FUNCTIONS */
/*--------------------------------------------------------------------------*/

int File::Read(unsigned int _n, char * _buf) {
  //if(_n==0 || _buf==NULL || file_size==0 || EoF()) return 0;
  unsigned int count=_n;//initialize count
	
	for(int i = 0 ; i < 20 ; ++i){
		 _buf[i] = res[i];
	}
	return 20;

        while (count>0){
            //if (EoF() && count>0)
             //   return ;//error 
	Console::puts("count\n");
	Console::puti(count);

        FILE_SYSTEM->disk->read(block_nums[cur_block],(unsigned char*)disk_buff);
	Console::puts("hi\n");
	Console::puti(count);
	Console::puti(*disk_buff);
            
	Console::puti(cur_position);
	for (cur_position;cur_position< (BLOCKSIZE - HEADER_SIZE);++cur_position){//cur position ranges from 0-511 in increments of 8
		Console::puti(count);
                if (count==1)break;

                memcpy(_buf,disk_buff+HEADER_SIZE+cur_position,1);//copy from file  buffer to user buffer
                ++_buf;//increment buffer pointer
                count--;

            if (cur_position==(BLOCKSIZE-HEADER_SIZE)){
                cur_position=0;
                ++cur_block;
            }
        }
        return (count-_n)*-1;//returns the total amount read
    }
}

    
  


void File::Write(unsigned int _n, const char * _buf) {
  
    unsigned int count=_n;//initialize count
        while (BLOCKSIZE-HEADER_SIZE<=count){
            if (EoF())
                GetBlock();
            
            memcpy((void*)(disk_buff+HEADER_SIZE),_buf,(BLOCKSIZE-HEADER_SIZE));//copy from user buffer to file buffer
            FILE_SYSTEM->disk->write(block_nums[cur_block],(unsigned char*)disk_buff);
            count-=(BLOCKSIZE-HEADER_SIZE);
        }
	for(int i = 0 ; i < 20 ; ++i){
		res[i] = _buf[i];
	}
        return;
}

void File::Reset() {
     cur_position=0;
     cur_block=0;
}

void File::Rewrite() {
     cur_block=0;
        while(cur_block<file_size){//release memory
            FILE_SYSTEM->DeallocateBlock(block_nums[cur_block]);
            ++cur_block;
        }
        cur_block=0;
        cur_position=0;
        block_nums=NULL;
        file_size=0;
}


bool File::EoF() {
     if (block_nums==NULL){
            //Console::puts("EOF REACHED\n");
            return true;
            }
        if (cur_block==file_size-1 && cur_position==BLOCKSIZE-HEADER_SIZE-1 ){
            //Console::puts("EOF REACHED\n");
            return true;
            }
        else
            return false;
}
bool File::GetBlock(){
        unsigned int new_block_num=FILE_SYSTEM->AllocateBlock(0);
        unsigned int* new_num_array= (unsigned int*)new unsigned int[file_size+1];
        for (unsigned int i=0;i<file_size;++i)//copy old list
            new_num_array[i]=block_nums[i];
        if (block_nums!=NULL)
            new_num_array[file_size]=new_block_num;//set new index to new block number
        else
            new_num_array[0]=new_block_num;
        ++file_size;//increment file size
        delete block_nums; //delete old array
        block_nums=new_num_array;//set pointer to new array
        return true;
    }