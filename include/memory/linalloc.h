
/**  Copyright 2007-2012 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#ifndef __LINALLOC_H__
#define __LINALLOC_H__

#include "c_vars.h"
#include "memory/memory.h"

//event linear buffer
typedef struct _LinearBuffer{
  U8 *pMemPtr;          // linear memory pointer start
  U32 totalSize;	// total linear memory size in bytes
  U32 offset;
  eMemoryFlag memType;  // memory type
} tLinearBuffer;

S32 createLinearBuffer(tLinearBuffer *buf, U32 bufferSize, eMemoryFlag memType);
void destroyLinearBuffer (tLinearBuffer *buf);

// non aligned allocation from linear buffer
void *linearBufferAlloc(tLinearBuffer *buf, U32 size);
void linearBufferFree(tLinearBuffer *buf); //clears all linear buffer

void *linearBufferAllocAlign(tLinearBuffer *buf, U32 size,U32 alignFlag);


#endif
