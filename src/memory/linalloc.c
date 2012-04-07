
#include "memory/linalloc.h"

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
