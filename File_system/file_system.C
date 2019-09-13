/*
     File        : file_system.C

     Author      : Riccardo Bettati
     Modified    : 2017/05/01

     Description : Implementation of simple File System class.
                   Has support for numerical file identifiers.
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
#include "file_system.H"


/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

FileSystem::FileSystem() {
    block_num=0;
    num_files=0;
    files=NULL;
    memset(disk_buff,0,BLOCKSIZE);//clear buffer

}

/*--------------------------------------------------------------------------*/
/* FILE SYSTEM FUNCTIONS */
/*--------------------------------------------------------------------------*/
 void FileSystem::push_back_file(File* newFile){
        if (files==NULL)
            files=newFile;
        else
        {
        File* new_file_array= (File*)new File[num_files+1];
        unsigned int i=0;
        for (i=0;i<num_files;++i)//copy old list
            new_file_array[i]=files[i];
        new_file_array[num_files+1]=*newFile;//set new index to new block number
        ++num_files;//increment files size
        delete files; //delete old array
        files=new_file_array;//set pointer to new array
   }
}
bool FileSystem::Mount(SimpleDisk * _disk) {
    
     disk=_disk;
	//Console::puts("b1");
        //disk->read(0a,disk_buff);
	//Console::puts("b3");
	
        num_files=block->size;
          for(unsigned int i = 0 ; i < num_files ; ++i ){ 
	    disk->read(0,disk_buff);//refresh buffer back to root node of file system
            File* newFile= new File();//create a new file
            disk->read(block->data[i],disk_buff);//puts file inode in buffer
            newFile->file_size=block->size;
            newFile->file_id=block->id;
            push_back_file(newFile);
	    	  
	}
        return true;
}

bool FileSystem::Format(SimpleDisk * _disk, unsigned int _size) {
   FILE_SYSTEM->setdisk(_disk);
   memset(disk_buff,0,BLOCKSIZE);//set entire disk to 0, automatically free memory
    
    for (int i=0;i<SYSTEM_BLOCKS;++i)
        _disk->write(i,disk_buff);
    block->availability=USED;//set block to used
    block->size=0;//write to  size block this will cause first 4 bytes to be empty which is interpereted as 0 files on disk
    _disk->write(0,disk_buff);//initializes master block
   
	return true;
}

File * FileSystem::LookupFile(int _file_id) {
      for (int i=0;i<num_files+1;++i){
	Console::puts("try\n");
	
	Console::puti(files[i].file_id);
	
	Console::puts("::");
	Console::puti(_file_id);
	Console::puts("\n");
	
            if (files[i].file_id==_file_id|| files[i].file_id+1 == _file_id){
		Console::puts("found\n");
                return &files[i];
            }
        }
    return NULL;
}
 bool FileSystem::LookupFile(unsigned int _file_id, File * _file){

        unsigned int i=0;
        for (i=0;i<num_files+1;++i){
            if (files[i].file_id==_file_id){
                *_file=files[i];
                return true;
            }
        }
        return false;
   }

bool FileSystem::CreateFile(int _file_id) {
    File* newFile=(File*) new File();

        if (LookupFile(_file_id,newFile)){
            return false;
            }

        newFile->file_id=_file_id;
//	Console::puts("In Create file\n");
//	Console::puti(_file_id);
        newFile->file_size=0;
        newFile->block_nums=NULL;
        newFile->Rewrite();//simply clears all data and sets fields to 0
        //do not need to handle allocating data, write function of file will take care of that
        //but we do need to create an inode
        //Console::putui(newFile->inode_block_num);
        newFile->inode_block_num=AllocateBlock(0);//get any free block
        disk->read(newFile->inode_block_num,disk_buff);//load block in buffer
        block->availability=USED;//set block to used
        block->size=0;//size 0
        block->id=_file_id;
        disk->write(newFile->inode_block_num,disk_buff);//write file inode to disk
        push_back_file(newFile);//pushback file object
        return true;
}
 bool FileSystem::remove_file(unsigned int _file_id){
        File* new_file_array= (File*)new File[num_files];
        bool found=false;
        for (unsigned int i=0;i<num_files;++i){//copy old list
            if (files[i].file_id==_file_id){
                found==true;
                files[i].Rewrite();//erases and unaalocated memory
                DeallocateBlock(files[i].inode_block_num);//deletes inode of file
                }
            if (!found){
                new_file_array[i]=files[i];
                --num_files;
                }
            else
                new_file_array[i]=files[i+1];
        }
        delete files; //delete old array
        files=new_file_array;//set pointer to new array
        if (num_files==0)
            files=NULL;//precautionary
        return found;
   }

bool FileSystem::DeleteFile(int _file_id) {
    File* oldFile;
        	remove_file(_file_id);
	return true;
}
/*
void FileSystem::Refresh(unsigned int block_no, unsigned char* _buf, unsigned int nextblock){
 while(1){
        if((int)block_no == -1){
            break;
        }
        File::file_system->disk->read(block_no,_buf);
        memcpy(&nextblock, _buf+508, 4);
        
        memcpy(_buf+508, &(File::file_system->freeblocks),4);
        File::file_system->disk->write(block_no,_buf);
        File::file_system->freeblocks = block_no;
        
        block_no = nextblock;
    }
}*/

 unsigned int FileSystem::AllocateBlock(unsigned int _block_num){
        if (_block_num!=0){//we assume the user is right
            disk->read(_block_num,disk_buff);
            block->availability=USED;//sets block header to used
            disk->write(_block_num,disk_buff);
            return _block_num;
        }
        else{//else we find a free block for them
            disk->read(block_num,disk_buff);
            int sanity_check=0;
            while (block->availability==USED){
                if (block_num>(SYSTEM_BLOCKS-1)){//look back at beginning
                    block_num=0;
                    ++sanity_check;
                    if (sanity_check>1){
                        Console::puts("ERROR NO FREE BLOCKS!!!!");
                        return 0;
                        }
                }
                ++block_num;
                disk->read(block_num,disk_buff);
            }
            disk->read(block_num,disk_buff);
            block->availability=USED;//sets block header to used
            disk->write(block_num,disk_buff);
            return block_num;
        }
   }
   /*Allocates a block from free list, for use in file*/
   void FileSystem::DeallocateBlock(unsigned int _block_num){
        disk->read(_block_num,disk_buff);
            block->availability=FREE;//sets block header to free
        disk->write(block_num,disk_buff);
   }