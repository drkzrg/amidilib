
/**  Copyright 2007-2012 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#ifndef __LINALLOC_H__
#define __LINALLOC_H__

#include "c_vars.h"
#include "memory/memory.h"

typedef struct _LinearBuffer{
  U8 *pMemPtr;          // linear memory pointer start
  U32 totalSize;	// total linear memory size in bytes
  U32 offset;
  eMemoryFlag memType;  // memory type
}tLinearBuffer;

// non aligned allocation from linear buffer
void *linearBufferAlloc(tLinearBuffer *buf, U32 size);


#endif
