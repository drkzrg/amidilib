
#include "memory/linalloc.h"
#include "mint/mintbind.h"
#include "memory.h"

static tLinearBuffer bufferInfo; 
S32 createLinearBuffer(U32 bufferSize, eMemoryFlag memType){
  
  bufferInfo.pMemPtr = amMallocEx(bufferSize,memType);
  
  if(bufferInfo.pMemPtr!=0){
    bufferInfo.memType=memType;
    bufferInfo.totalSize=bufferSize;
  return 0L;
 } 
 
 return -1L;
}

void destroyLinearBuffer(){
  amFree((void**)&bufferInfo);
}

// non aligned allocation from linear buffer
void *linearBufferAlloc(tLinearBuffer *buf, U32 size){
  if(!buf||!size) return NULL;
  
  U32 newOffset=buf->offset+size;
  
  if(newOffset<=buf->totalSize){
      void *ptr=buf->pMemPtr+buf->offset;
      buf->offset=newOffset;
      return ptr;
  }
  
  return NULL; //out of memory
}

// non aligned allocation from linear buffer
void *linearBufferAllocAlign(tLinearBuffer *buf, U32 size,U32 alignFlag){
  
}
