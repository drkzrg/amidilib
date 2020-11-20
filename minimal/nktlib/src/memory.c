
/**  Copyright 2007-2020 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

// atari TOS version
#include <stdlib.h>
#include "memory/memory.h"

// global memory allocation callbacks
funcMemAlloc gUserMemAlloc;
funcMemFree gUserMemFree;
funcMemRealloc gUserMemRealloc;

#if defined (FORCE_MALLOC)

void* amMalloc(unsigned int size, short int ramflag, void *param)
{
    return malloc(size);
}

void amFree(void *ptr, void *param)
{
   AssertMsg(ptr!=NULL,"free() called on NULL address!"); 
   free(ptr); 
}

#else

#include <mint/osbind.h>

void* amMalloc(unsigned int size, short int memflag, void *param)
{
    return (void *)Malloc(size);
}

void* amMallocEx(unsigned int size, short int memflag, void *param)
{
    return (void *)Mxalloc(size,(short int)memflag);
}

void amFree(void *ptr, void *param)
{
    AssertMsg(ptr!=NULL,"MFree() called on NULL address!"); 
    Mfree(ptr);
}
#endif

void* amRealloc( void *pPtr, const MemSize newSize, void *param)
{
 return realloc(pPtr,newSize);
}

// sets default memory allocation functions
void amSetDefaultUserMemoryCallbacks(void)
{

#ifdef TARGET_ST    
    gUserMemAlloc = (funcMemAlloc)amMalloc;
#else
    gUserMemAlloc = (funcMemAlloc)amMallocEx;
#endif

    gUserMemFree = (funcMemFree)amFree;
    gUserMemRealloc = (funcMemRealloc)amRealloc;
}

// sets user memory allocation functions
void amSetUserMemoryCallbacks(sUserMemoryCallbacks *func)
{
    AssertMsg(func != NULL,"Fatal error: Memory callback struct is null!"); 
    
    if(gUserMemAlloc!=0) gUserMemAlloc = func->cbUserMemAlloc;
    if(gUserMemFree!=0) gUserMemFree = func->cbUserMemFree;
    if(gUserMemRealloc!=0) gUserMemRealloc = func->cbMemRealloc;
}

// writes a byte, and returns the buffer
uint8* WriteByte(void* buf, int8 b)
{
    uint8* buffer = (uint8*)buf;
    *buffer = b;
    ++buffer;
    return buffer;
}

uint8* WriteShort(void* b, uint16 s)
{
    uint16* buffer = (uint16*)b;
    *buffer = s;
    ++buffer;
    return (uint8 *)buffer;
}

uint8* WriteInt(void* b, uint32 i)
{
 uint32* buffer = (uint32*)b;
 *buffer = i;
 ++buffer;
 return (uint8 *)buffer;
}

int32 UpdateBytesWritten(int32* bytes_written, int32 to_add, int32 max)
{
    *bytes_written += to_add;

    if(max!=-1){
        if (max && *bytes_written > max){
            return 0;
        }
    } 
    
  return 1;
}
