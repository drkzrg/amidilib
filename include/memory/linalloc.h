
/**  Copyright 2007-2021 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#ifndef LINALLOC_H_
#define LINALLOC_H_

#include "c_vars.h"
#include "memory/memory.h"

// event linear buffer
typedef struct LINEARBUFFERALLOCATOR
{
  void* bufferStart;    	// linear memory buffer start
  MemSize size;        		// total memory size in bytes
  uint32 offset;
  eMemoryFlag memType;  	// memory type
} LinearBufferAllocator;

int32 createLinearBuffer(LinearBufferAllocator *allocatorState, const MemSize bufferSize, const eMemoryFlag memType);
void destroyLinearBuffer (LinearBufferAllocator *allocatorState);

// non aligned allocation from linear buffer
void *linearBufferAlloc(LinearBufferAllocator *allocatorState, const MemSize size);
void linearBufferFree(LinearBufferAllocator *allocatorState);
void *linearBufferAllocAlign(LinearBufferAllocator *allocatorState, const MemSize allocSize, const uint32 alignFlag);
void linearBufferPrintInfo(const LinearBufferAllocator *allocatorState);

#endif
