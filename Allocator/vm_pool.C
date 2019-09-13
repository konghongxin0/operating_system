/*
 File: vm_pool.C
 
 Author:hongxin kong
 Date  :03/18/2019
 
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
#include "page_table.H"
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
               PageTable     *_page_table):_base_address(_base_address),size (_size)
              ,frame_pool (_frame_pool),page_table(_page_table){
    num_regions=0;
    max_regions= Machine::PAGE_SIZE/sizeof(region_info);
    regions = (region_info*)(Machine::PAGE_SIZE * (frame_pool->get_frames(1)));
    page_table -> register_pool(this);
    Console::puts("Constructed VMPool object.\n");
}

unsigned long VMPool::allocate(unsigned long _size) {
    if(_size==0)
            return 0;//failed allocation because size was 0

    unsigned long start = 0; //where the starting address of the new region will be
    if(num_regions==0)// if allocated list is empty
        start = _base_address;
    else
        start = regions[num_regions-1].base_address + regions[num_regions-1].size;
    
    regions[num_regions].base_address = start;
    regions[num_regions].size = _size;
     ++num_regions;
     if(num_regions>max_regions){
                Console::puts("out of space");
                for(;;);
      }
      return start;
    Console::puts("Allocated region of memory.\n");
}

void VMPool::release(unsigned long _start_address) {
    unsigned int index=0;
        for(unsigned int i; i < num_regions; i++)
            if(regions[i].base_address == _start_address){
                index=i;
                break;
            }

        for (unsigned int j = 0; j < regions[index].size/Machine::PAGE_SIZE; j++)
        {
            page_table->free_page(_start_address);
            _start_address = _start_address + PageTable::PAGE_SIZE;
        }
        //restructure array

        region_info* old= regions;
        regions=  (region_info*)(Machine::PAGE_SIZE * (frame_pool->get_frames(1)));
        unsigned int new_count=0;
        for (unsigned int k=0;k<num_regions;++k){
            if (k!=index)
                regions[new_count]=old[k];
            ++new_count;
        }
        frame_pool->release_frames((unsigned long)old/Machine::PAGE_SIZE);
        // flush the TLB
        page_table->load();


    Console::puts("Released region of memory.\n");
}

bool VMPool::is_legitimate(unsigned long _address) {
    

for(unsigned long i = 0; i < this->num_regions; i++) {
		unsigned long region_boundry = this->regions[i].base_address + this->regions[i].size;
		if(_address >= this->regions[i].base_address && _address <= region_boundry) {
			return 1;
		}
	}
	return 0;
    Console::puts("checked whether address is part of an allocated reigon.\n");

}
