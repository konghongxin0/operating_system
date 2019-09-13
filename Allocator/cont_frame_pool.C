/*
 File: ContFramePool.C
 
 Author:hongxin kong
 Date  : 03/20/2019
 
 */

/*--------------------------------------------------------------------------*/
/* 
 POSSIBLE IMPLEMENTATION
 -----------------------

 The class SimpleFramePool in file "simple_frame_pool.H/C" describes an
 incomplete vanilla implementation of a frame pool that allocates 
 *single* frames at a time. Because it does allocate one frame at a time, 
 it does not guarantee that a sequence of frames is allocated contiguously.
 This can cause problems.
 
 The class ContFramePool has the ability to allocate either single frames,
 or sequences of contiguous frames. This affects how we manage the
 free frames. In SimpleFramePool it is sufficient to maintain the free 
 frames.
 In ContFramePool we need to maintain free *sequences* of frames.
 
 This can be done in many ways, ranging from extensions to bitmaps to 
 free-lists of frames etc.
 
 IMPLEMENTATION:
 
 One simple way to manage sequences of free frames is to add a minor
 extension to the bitmap idea of SimpleFramePool: Instead of maintaining
 whether a frame is FREE or ALLOCATED, which requires one bit per frame, 
 we maintain whether the frame is FREE, or ALLOCATED, or HEAD-OF-SEQUENCE.
 The meaning of FREE is the same as in SimpleFramePool. 
 If a frame is marked as HEAD-OF-SEQUENCE, this means that it is allocated
 and that it is the first such frame in a sequence of frames. Allocated
 frames that are not first in a sequence are marked as ALLOCATED.
 
 NOTE: If we use this scheme to allocate only single frames, then all 
 frames are marked as either FREE or HEAD-OF-SEQUENCE.
 
 NOTE: In SimpleFramePool we needed only one bit to store the state of 
 each frame. Now we need two bits. In a first implementation you can choose
 to use one char per frame. This will allow you to check for a given status
 without having to do bit manipulations. Once you get this to work, 
 revisit the implementation and change it to using two bits. You will get 
 an efficiency penalty if you use one char (i.e., 8 bits) per frame when
 two bits do the trick.
 
 DETAILED IMPLEMENTATION:
 
 How can we use the HEAD-OF-SEQUENCE state to implement a contiguous
 allocator? Let's look a the individual functions:
 
 Constructor: Initialize all frames to FREE, except for any frames that you 
 need for the management of the frame pool, if any.
 
 get_frames(_n_frames): Traverse the "bitmap" of states and look for a 
 sequence of at least _n_frames entries that are FREE. If you find one, 
 mark the first one as HEAD-OF-SEQUENCE and the remaining _n_frames-1 as
 ALLOCATED.

 release_frames(_first_frame_no): Check whether the first frame is marked as
 HEAD-OF-SEQUENCE. If not, something went wrong. If it is, mark it as FREE.
 Traverse the subsequent frames until you reach one that is FREE or 
 HEAD-OF-SEQUENCE. Until then, mark the frames that you traverse as FREE.
 
 mark_inaccessible(_base_frame_no, _n_frames): This is no different than
 get_frames, without having to search for the free sequence. You tell the
 allocator exactly which frame to mark as HEAD-OF-SEQUENCE and how many
 frames after that to mark as ALLOCATED.
 
 needed_info_frames(_n_frames): This depends on how many bits you need 
 to store the state of each frame. If you use a char to represent the state
 of a frame, then you need one info frame for each FRAME_SIZE frames.
 
 A WORD ABOUT RELEASE_FRAMES():
 
 When we releae a frame, we only know its frame number. At the time
 of a frame's release, we don't know necessarily which pool it came
 from. Therefore, the function "release_frame" is static, i.e., 
 not associated with a particular frame pool.
 
 This problem is related to the lack of a so-called "placement delete" in
 C++. For a discussion of this see Stroustrup's FAQ:
 http://www.stroustrup.com/bs_faq2.html#placement-delete
*/
/*--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "cont_frame_pool.H"
#include "console.H"
#include "utils.H"
#include "assert.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/
ContFramePool* ContFramePool::pool_list;

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   C o n t F r a m e P o o l */
/*--------------------------------------------------------------------------*/

 static unsigned char MASK[] = {128, 64, 32, 16, 8, 4, 2, 1};

    bool IsNthbit (unsigned char& c, int n) {
        return ((c & MASK[n]) == 0);
    }
    void SetNthbit(unsigned char& c, int n){
            c = c|MASK[n];
    }
    void ClearNthbit(unsigned char& c, int n){
            c= c&(~MASK[n]);
    }


ContFramePool::ContFramePool(unsigned long _base_frame_no,
                             unsigned long _n_frames,
                             unsigned long _info_frame_no,
                             unsigned long _n_info_frames){
    // TODO: IMPLEMENTATION NEEEDED!

    /*Constructor: Initialize all frames to FREE, except for any frames that you 
      need for the management of the frame pool, if any.
    */
    assert(_n_info_frames<=_n_frames)
    
    base_frame_no = _base_frame_no;
    nframes = _n_frames;
    nFreeFrames = _n_frames;
    info_frame_no = _info_frame_no;
    n_info_frames = _n_info_frames;
    
    // If _info_frame_no is zero then we keep management info in the first
    //frame, else we use the provided frame to keep management info
    if(info_frame_no == 0) {
        bitmap = (unsigned char *) (base_frame_no * FRAME_SIZE);
        avamap = bitmap+1024;//store second map 1024 bytes later
        memset(bitmap,0,FRAME_SIZE); //intial
        memset(bitmap,0x80,1);//sets first bit to 1 if info_frame_no == 0 
        info_frame_no=base_frame_no;//store new info_frame_number
    } 
    else {
         bitmap = (unsigned char *) (info_frame_no * FRAME_SIZE);
         avamap = bitmap+1024;//store second map 1024 bytes later
         memset(avamap,0,FRAME_SIZE); // initial
    }
    
    // Everything ok. Proceed to mark all bits in the bitmap huge nut okey for now
    
    assert ((nframes % 8 ) == 0);

     if (ContFramePool::pool_list==NULL)
                ContFramePool::pool_list=this;
     else
     	pool_list->next = this;
    
     prev = pool_list;
    Console::puts("Frame Pool initialized\n");
}

unsigned long ContFramePool::get_frames(unsigned int _n_frames)
{
    unsigned int i=0;
    int space =0;
     for (;i<nframes/8;++i){ //access bitmap 1 byte at a time
            if(bitmap[i]^0xFF!=0){ // some bit availible
                for (int j=0;j<8;++j){ 
                    if(IsNthbit(bitmap[i],j) && IsNthbit(avamap[i],j))//linear probe block to find empty page
                    {
                    if(_n_frames ==1){
                       SetNthbit(bitmap[i],j);
                       SetNthbit(avamap[i],j);
                       return base_frame_no+i*8+j;//return frame number 
		    } 
		       int k = j;
			for(; k <8 && space < _n_frames;++k){
                            if (!(IsNthbit(bitmap[i],k) && IsNthbit(avamap[i],k)))
                                break;
                            else
                                space+=1;
                        }
                        if(k==8&&space<_n_frames){
                          int l = 0; 
			  for(; l <8 && space < _n_frames ;++l){
                              if(IsNthbit(bitmap[i+1],l) && IsNthbit(avamap[i+1],l)){
                                    space+=1;
                                if (space == _n_frames){
				int lm = 0;
                                    for(; lm<= l ;++lm){
                                        SetNthbit(bitmap[i+1],lm);
                                    }
                                    for(int mark = i*8+j ; mark < i*8+j+_n_frames-lm;++mark){
                                      if(mark == i*8+j){
                                        SetNthbit(bitmap[i],mark-(i*8));
                                        SetNthbit(avamap[i],mark-(i*8)); //mark frame as used
                                      }
                                      else 
                                        SetNthbit(bitmap[i],mark-(i*8)); //mark frame as used

                                    }
                                    return base_frame_no+i*8+j;//return frame number 
                                }
                              }
                              else 
                                break;
                            }
                        }
                        else if(space == _n_frames){
                            for(int mark = i*8+j ; mark < i*8+j+_n_frames;++mark){
                                      if(mark == i*8+j){
                                        SetNthbit(avamap[i],mark-(i*8));
                                        SetNthbit(bitmap[i],mark-(i*8)); //mark frame as used
                                      }
                                      else 
                                        SetNthbit(bitmap[i],mark-(i*8)); //mark frame as used
                            }
                            return base_frame_no+i*8+j;//return frame number 
                        }
                    }
                }
            }
    }

    Console::puts("Err no free frames found\n");
    return 0;
    
}

void ContFramePool::mark_inaccessible(unsigned long _base_frame_no,
                                      unsigned long _n_frames)
{
if(_base_frame_no<base_frame_no||_base_frame_no+_n_frames>=base_frame_no+nframes)
            Console::puts("Err cannot mark frames inaccessible, out of range\n");
        else
            {
            memset(avamap,0xFF,_n_frames/8); //sets all but the last few bits as used
            for(unsigned int i=0;i<_n_frames%8;++i){
                SetNthbit(avamap[_n_frames/8],i);//sets remainder bits as used
                }
            }

}

void ContFramePool::release_frames(unsigned long _first_frame_no)
{
 //	Console::puts("In release frame\n");
	ContFramePool* curr=ContFramePool::pool_list;
 
       //determine if frame is in curr
        if (curr->base_frame_no+ curr->nframes <= _first_frame_no)
        {
            if(curr->next!=NULL)
                curr=curr->next;
            else{
                Console::puts("Error releasing frame, was not in any frame pools\n");
                assert(false);
            }
        }
       
        unsigned char* frame_byte = &curr->bitmap[(_first_frame_no-curr->base_frame_no)/8];//gets byte containing frame
        unsigned char* next_frame_byte = &curr->bitmap[(_first_frame_no-curr->base_frame_no)/8+1];
        unsigned char* ava_byte = &curr->avamap[(_first_frame_no-curr->base_frame_no)/8+1];
        unsigned char* next_ava_byte = &curr->avamap[(_first_frame_no-curr->base_frame_no)/8];
         ClearNthbit(*frame_byte,(_first_frame_no-(curr->base_frame_no))%8);
         ClearNthbit(*ava_byte,(_first_frame_no-(curr->base_frame_no))%8);
         int k = (_first_frame_no-(curr->base_frame_no))%8+1;
/*
        bool flag = true;
        while(flag){
            if
        }
  */
        if(k==8){
          k = 0; 
          frame_byte= next_frame_byte;
          ava_byte = next_ava_byte;
         }
   for (int i = 0 ; i < 4 ; ++i){
     if( !IsNthbit(*frame_byte,k) && !IsNthbit(*ava_byte,k)){
        break;
     }
     else{
        if(!IsNthbit(*frame_byte,k))
         ClearNthbit(*frame_byte,k);
        else if( !IsNthbit(*ava_byte,k))
         ClearNthbit(*ava_byte,k);
     
     k = k+1;
     if(k==8){
          k = 0; 
          frame_byte= next_frame_byte;
          ava_byte = next_ava_byte;
         }
     }
   }      

        
}

unsigned long ContFramePool::needed_info_frames(unsigned long _n_frames)
{
    // TODO: IMPLEMENTATION NEEEDED!
    // one char each
   return  _n_frames/8 +(_n_frames % 8 > 0 ? 1 : 0);
}
