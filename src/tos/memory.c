
/**  Copyright 2007-2022 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#include <stdlib.h>

#include "memory/memory.h"
#include "ct60/ct60.h"
#include "core/machine.h"

// global memory allocation callbacks
funcMemAlloc gUserMemAlloc;
funcMemFree gUserMemFree;
funcMemRealloc gUserMemRealloc;

#if defined (FORCE_MALLOC)

void* amMalloc(const uint32_t size, const eMemoryFlag ramflag, void *param)
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

void* amMalloc(const uint32_t size, const eMemoryFlag ramflag, void *param)
{
  void *pMem = 0;
  
  const sMachineInfo * const info = getMachineInfo();

  if (info->gemdos_ver >= 0x1900)
  {
    pMem = (void *)Mxalloc(size, ramflag);
  }
  else
  {
    pMem = (void *)Malloc(size);
  }

  return pMem;
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
    gUserMemAlloc = (funcMemAlloc)amMalloc;
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

uint32 amGetFreeMemory(const eMemoryFlag memFlag) 
{
  AssertMsg(memFlag != MF_SUPERVIDELRAM,"SuperVidel memory not supported yet [TODO].");

  void *pMem = 0;
  const sMachineInfo * const info = getMachineInfo();

  if (info->gemdos_ver >= 0x1900) 
  {
    if(memFlag == MF_SUPERVIDELRAM)
    { 
      // TODO: more hardware checks
      ct60_vmalloc(-1,0);
    }
    else
    {
      pMem = (void *)Mxalloc(-1L, memFlag);
    }

  } 
  else 
  {
    // no fastram at all
    if (memFlag == MF_PREFER_FASTRAM)
    {
      pMem = (void *)0;
    }
    else
    {
      pMem = (void *)Malloc(-1L);
    }
  }

  return ((uint32)pMem);
}

