#include "assert.H"
#include "exceptions.H"
#include "console.H"
#include "paging_low.H"
#include "page_table.H"
PageTable * PageTable::current_page_table = NULL;
unsigned int PageTable::paging_enabled = 0;
ContFramePool * PageTable::kernel_mem_pool = NULL;
ContFramePool * PageTable::process_mem_pool = NULL;
unsigned long PageTable::shared_size = 0;

void PageTable::init_paging(ContFramePool * _kernel_mem_pool,
                            ContFramePool * _process_mem_pool,
                            const unsigned long _shared_size)
{
    PageTable::kernel_mem_pool = _kernel_mem_pool; //pointers
    PageTable::process_mem_pool = _process_mem_pool;
    PageTable::shared_size = _shared_size;//size of shared kernel memory

   Console::puts("Initialized Paging System\n");
}
unsigned long PageTable::get_page_directory(){
	return (unsigned long) page_directory;
}

PageTable::PageTable()
{
   page_directory = ( unsigned long*)(process_mem_pool->get_frames(1)*PAGE_SIZE);
   unsigned long* page_table= (unsigned long*)(process_mem_pool->get_frames(1)*PAGE_SIZE);
   unsigned long address=0; // holds the physical address of where a page is
   memset(page_directory,0,PAGE_SIZE);
   // map the first 4MB of memory for page table
    for(unsigned int i=0; i<1024; i++){
      page_table[i] = address | 3; // attribute set to: supervisor level, read/write, present(011 in binary)
      address = address + PAGE_SIZE ; // 4096 = 4kb
    }

    page_directory[0] = (unsigned long) page_table;
    page_directory[0] |= 3;

    for(unsigned int i=1; i<1023; i++){
      page_directory[i] = 0 | 2; // attribute set to: supervisor level, read/write, not present(010 in binary)
    };
    page_directory[1023]=(unsigned long)page_directory | 3;

    num_registered_vmpools = 0;

    for(unsigned int i=0; i<MAX_VM; i++){
        registered_vmpools[i] = NULL;
    }

   Console::puts("Constructed Page Table object\n");
}


void PageTable::load()
{
   current_page_table  = this;
   Console::puts("Loaded page table\n");
}

void PageTable::enable_paging()
{
    write_cr3(current_page_table->get_page_directory());
    paging_enabled = 1;
    write_cr0(read_cr0() | 0x80000000);

       Console::puts("Enabled paging\n");
}

void PageTable::handle_fault(REGS * _r)
{

    unsigned long address = read_cr2();
    unsigned long err_code = _r->err_code;
    unsigned long *new_page_table;


    if((err_code&1) == 0){ //not present fault 
         VMPool** vm_head = current_page_table->registered_vmpools;
         int vm_index=-1;
	 for(unsigned int i=0;i<current_page_table->num_registered_vmpools;++i){
            if(vm_head[i]!=NULL){
                if (vm_head[i]->is_legitimate(address)){
                    vm_index=i;
                    break;
                }
            }
          }

          unsigned long* dir_address =(unsigned long*) (((address >> 22) << 2 ) | ((0xFFFFF)<<12));//1023 =  2^10-1
          unsigned long* page_address =(unsigned long*) (((address >> 12) << 2 ) | ((0x03FF)<<22));//1048575 = 2^20-1
          //switch off the bit that we don't need

          if(vm_head[vm_index] != NULL){
                if((*dir_address & 1) == 1){
                  *page_address = PageTable::process_mem_pool->get_frames(1);
                  *page_address = (*page_address << 12) | 5;
                }
                else{
                   unsigned long tmp = PageTable::process_mem_pool->get_frames(1);
                   *dir_address = tmp;
                   *dir_address =(( *dir_address) << 12) | 3;

                  address = (address>>22)<<22;
                  for(int i=0; i<1024; i++){
                      unsigned long* temp_addr = (unsigned long*) (((address >> 12) << 2 ) | ((0x03FF)<<22));
                      *temp_addr = 4;
                      address = ((address>>12)+1)<<12;
                  }

                  *page_address = PageTable::process_mem_pool->get_frames(1);
                  *page_address=  ((*page_address) << 12) | 3;

                }
          }
    
      }
    
  Console::puts("handled page fault\n");
}

void PageTable::register_pool(VMPool *_pool) {
  if((this->registered_vmpools[this->num_registered_vmpools]==NULL)
    &&(num_registered_vmpools<MAX_VM)){
    this->registered_vmpools[this->num_registered_vmpools] = _pool;
    this->num_registered_vmpools++;
  }
  else
     Console::puts("Action fail\"wrong VM pool\", can be caused by register too many pool\n");
  Console::puts("registered VM pool\n");
}

void PageTable::free_page(unsigned long _page_no) {

    unsigned long PhysicalFrameNo;
    unsigned long * PageTableEntryPtr;
    PageTableEntryPtr = (unsigned long *) (((_page_no >> 12) << 2 ) | ((0x03FF)<<22));
    PhysicalFrameNo = (*PageTableEntryPtr) >> 12;
    *PageTableEntryPtr = 0;
    process_mem_pool->release_frames(PhysicalFrameNo);
    Console::puts("freed page\n");



}
