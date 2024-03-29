
#include "core/assert.h"
#include "memory/linalloc.h"
#include "core/amprintf.h"

static const uint32 DEBUGFILL = 0x000000FFUL;

void linearBufferPrintInfo(const LinearBufferAllocator *allocatorState)
{
  AssertMsg(allocatorState != 0, "Linear buffer not initialised or corrupted!");
  amTrace("LinearBuffer buffer start: %p, size: %ld, current offset: %ld"NL,allocatorState->bufferStart, allocatorState->size, allocatorState->offset);
}

int32 createLinearBuffer(LinearBufferAllocator *allocatorState, const MemSize bufferSize, const eMemoryFlag memType)
{

  AssertMsg(allocatorState != 0, "Linear buffer not initialised or corrupted!");
  AssertMsg(bufferSize > 0UL, "Linear buffer size must be grater than 0!");
  
  allocatorState->bufferStart = gUserMemAlloc(bufferSize, memType, 0);
  
  if(allocatorState->bufferStart != 0 )
  {

    #ifdef DEBUG_MEM
      amMemSet(allocatorState->bufferStart, DEBUGFILL, bufferSize);
    #endif      

    allocatorState->memType = memType;
    allocatorState->size = bufferSize;
    allocatorState->offset = 0L;
    
    return 0L;
 } 
 
 return -1L;
}

void destroyLinearBuffer(LinearBufferAllocator *allocatorState)
{
  AssertMsg(allocatorState != 0, "Linear buffer not initialised or corrupted!");
  AssertMsg(allocatorState->bufferStart != 0, "bufferStart is 0!");

  switch(allocatorState->memType)
  {
    //release memory block depending on type
    case MF_STRAM:
    case MF_FASTRAM:
    case MF_PREFER_STRAM:
    case MF_PREFER_FASTRAM:
    {
#ifdef DEBUG_MEM
      amMemSet(allocatorState->bufferStart, DEBUGFILL, allocatorState->size);
#endif 
      gUserMemFree(allocatorState->bufferStart,0);
    } break;
    case MF_SUPERVIDELRAM:
    default:
    {
      AssertMsg(false,"Invalid / unsupported memory type passed!");
    } break;
  };
  
  allocatorState->bufferStart = 0UL;
  allocatorState->size = 0UL;	
  allocatorState->offset = 0UL;
}

// non aligned allocation from linear allocatorStatefer
void *linearBufferAlloc(LinearBufferAllocator *allocatorState, const MemSize size)
{
  AssertMsg(allocatorState != 0, "Linear buffer not initialised or corrupted!");
  AssertMsg(size > 0UL, "Allocation size cannot be 0!");
  
  uint32 newOffset = allocatorState->offset + size;
  void* addr = NULL;

  if(newOffset <= allocatorState->size)
  {
      addr = (void *)(((uintptr)allocatorState->bufferStart) + allocatorState->offset);
      allocatorState->offset = newOffset;
  }
  
  return addr; 
}

// non aligned allocation from linear buffer (TODO)
void *linearBufferAllocAlign(LinearBufferAllocator *allocatorState, const MemSize size, const uint32 alignFlag)
{
  AssertMsg(allocatorState != 0, "Linear buffer not initialised or corrupted!");
  AssertMsg(size > 0UL, "Allocation size cannot be 0!");

  const uint32 newOffset = allocatorState->offset + size;
  void* addr = NULL;

  if(newOffset <= allocatorState->size)
  {
    addr = (void*)(((uintptr)allocatorState->bufferStart) + allocatorState->offset);
    allocatorState->offset = newOffset;
  }
  
  return addr; 
}

void linearBufferFree(LinearBufferAllocator *allocatorState)
{
   AssertMsg(allocatorState != 0, "Linear buffer not initialised or corrupted!");
   AssertMsg(allocatorState->bufferStart != 0, "bufferStart is 0!");

   switch(allocatorState->memType)
   {
    //clean memory for whole block
    case MF_STRAM:
    case MF_FASTRAM:
    case MF_PREFER_STRAM:
    case MF_PREFER_FASTRAM:
    {
#ifdef DEBUG_MEM
        amMemSet(allocatorState->bufferStart,DEBUGFILL,(MemSize)allocatorState->size);
#endif      
    } break;
    case MF_SUPERVIDELRAM:
    default:
    {
        AssertMsg(false,"Invalid / unsupported memory type passed!");
    }break;
  }
  
  allocatorState->offset = 0UL;
}
