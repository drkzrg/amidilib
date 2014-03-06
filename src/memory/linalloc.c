
#include "memory/linalloc.h"

S32 createLinearBuffer(tLinearBuffer *buf, U32 bufferSize, eMemoryFlag memType){
  return 0;
}

void destroyLinearBuffer (tLinearBuffer *buf){}

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


void *linearBufferAllocAlign (tLinearBuffer *buf, U32 size,U32 alignFlag){
  if(!buf||!size) return NULL;
  
  U32 newOffset=buf->offset+size;
  
  if(newOffset<=buf->totalSize){
      void *ptr=buf->pMemPtr+buf->offset;
      buf->offset=newOffset;
      return ptr;
  }
  
  return NULL; //out of memory
}


