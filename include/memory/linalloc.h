
/**  Copyright 2007-2012 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#ifndef __LINALLOC_H__
#define __LINALLOC_H__

#include "c_vars.h"
#include "memory/memory.h"

// event linear buffer
typedef struct _LinearBuffer{
  uint8 *pMemPtr;          // linear memory pointer start
  uint32 totalSize;        // total linear memory size in bytes
  uint32 offset;
  eMemoryFlag memType;  // memory type
} tLinearBuffer;

int32 createLinearBuffer(tLinearBuffer *buf,const uint32 bufferSize,const eMemoryFlag memType);
void destroyLinearBuffer (tLinearBuffer *buf);

// non aligned allocation from linear buffer
void *linearBufferAlloc(tLinearBuffer *buf, const uint32 size);
void linearBufferFree(tLinearBuffer *buf); //clears all linear buffer
void *linearBufferAllocAlign(tLinearBuffer *buf, const uint32 size,const uint32 alignFlag);
void linearBufferPrintInfo(const tLinearBuffer *buf);


#endif
