
/**  Copyright 2007-2020 Pawel Goralski
    
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
  uint8 *pMemPtr;           // linear memory pointer start
  uint32 totalSize;         // total linear memory size in bytes
  uint32 offset;
  eMemoryFlag memType;  	// memory type
} LinearBufferAllocator;

int32 createLinearBuffer(LinearBufferAllocator *buf, const uint32 bufferSize, const eMemoryFlag memType);
void destroyLinearBuffer (LinearBufferAllocator *buf);

// non aligned allocation from linear buffer
void *linearBufferAlloc(LinearBufferAllocator *buf, const uint32 size);
void linearBufferFree(LinearBufferAllocator *buf); 			
void *linearBufferAllocAlign(LinearBufferAllocator *buf, const uint32 size, const uint32 alignFlag);
void linearBufferPrintInfo(const LinearBufferAllocator *buf);

#endif
