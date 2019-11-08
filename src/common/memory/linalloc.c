
#include "memory/linalloc.h"
#include "mintbind.h"

#include "memory.h"
#include "amlog.h"

void linearBufferPrintInfo(const tLinearBuffer *buf){
  if(buf==NULL) return;
  amTrace("LB memPtr: %p, size: %d, type: %d, offset: %d\n",buf->pMemPtr, buf->totalSize,buf->memType,buf->offset);
}

int32 createLinearBuffer(tLinearBuffer *buf, const uint32 bufferSize,const eMemoryFlag memType){
  
  buf->pMemPtr = amMallocEx(bufferSize,memType);
  
  if(buf->pMemPtr!=0&&bufferSize>0){
    buf->memType=memType;
    buf->totalSize=bufferSize;
    buf->offset=0L;
    
    #ifdef DEBUG_MEM
      amMemSet(buf->pMemPtr,0xDEADBEEF,(MemSize)buf->totalSize);
    #else
      amMemSet(buf->pMemPtr,0L,(MemSize)buf->totalSize);
    #endif      
    
  return 0L;
 } 
 
  buf->pMemPtr=0;
  buf->totalSize=0L;	
  buf->offset=0L;
 
 return -1L;
}

void destroyLinearBuffer(tLinearBuffer *buf){
  
  switch(buf->memType){
    //release memory block depending on type
    case ST_RAM:
    case TT_RAM:
    case PREFER_ST:
    case PREFER_TT:{
#ifdef DEBUG_MEM
      if(buf->pMemPtr!=0) amMemSet(buf->pMemPtr,0xDEADBEEF,(MemSize)buf->totalSize);
#else
      if(buf->pMemPtr!=0) amMemSet(buf->pMemPtr,0L,(MemSize)buf->totalSize);
#endif 
      if(buf->pMemPtr!=0) amFree(buf->pMemPtr);
    }break;
    case PREFER_DSP:
    case PREFER_SUPERVIDEL:
    case PREFER_RADEON:
    default:{;}break;
  }
  
  buf->pMemPtr=0;
  buf->totalSize=0L;	
  buf->offset=0L;
}

// non aligned allocation from linear buffer
void *linearBufferAlloc(tLinearBuffer *buf, const uint32 size){

  if(!buf||!size) return NULL;
  
  uint32 newOffset=buf->offset+size;
  
  if(newOffset<=buf->totalSize){
      void *ptr=buf->pMemPtr+buf->offset;
      buf->offset=newOffset;
      return ptr;
  }
  
  return NULL; //out of memory
}

// non aligned allocation from linear buffer (TODO)
void *linearBufferAllocAlign(tLinearBuffer *buf, const uint32 size,const uint32 alignFlag){
  if(!buf||!size) return NULL;
  
  uint32 newOffset=buf->offset+size;
  
  if(newOffset<=buf->totalSize){
      void *ptr=buf->pMemPtr+buf->offset;
      buf->offset=newOffset;
      return ptr;
  }
  
  return NULL; //out of memory
}

void linearBufferFree(tLinearBuffer *buf){
   
   switch(buf->memType){
    //clean memory for whole block
    case ST_RAM:
    case TT_RAM:
    case PREFER_ST:
    case PREFER_TT:{
#ifdef DEBUG_MEM
      if(buf->pMemPtr!=0) amMemSet(buf->pMemPtr,0xDEADBEEF,(MemSize)buf->totalSize);
#else
      if(buf->pMemPtr!=0) amMemSet(buf->pMemPtr,0L,(MemSize)buf->totalSize);
#endif      
    }break;
    case PREFER_DSP:
    case PREFER_SUPERVIDEL:
    case PREFER_RADEON:
    default:{;}break;
  }
  
  buf->offset=0L;
}

