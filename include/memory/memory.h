
/**  Copyright 2007-2022 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#ifndef __AMEMORY_TOS_H__
#define __AMEMORY_TOS_H__

#include "vartypes.h"
#include "core/logger.h"

#if AMIDILIB_USE_LIBC
#include <string.h>
#else
#include "amstring.h"
#endif

/* memory allocation preference */

#if defined (FORCE_MALLOC)
typedef enum EMEMORYFLAG 
{
  MF_STRAM,
  MF_FASTRAM,
  MF_PREFER_STRAM,
  MF_PREFER_FASTRAM,
  MF_SUPERVIDELRAM = 66
} eMemoryFlag;

#else

#include <mint/ostruct.h>

typedef enum EMEMORYFLAG 
{
  MF_STRAM = MX_STRAM,
  MF_FASTRAM = MX_TTRAM,
  MF_PREFER_STRAM = MX_PREFSTRAM,
  MF_PREFER_FASTRAM = MX_PREFTTRAM,
  MF_SUPERVIDELRAM = 66
} eMemoryFlag;

#endif

/* memory callback types */
typedef void *(*funcMemAlloc)(const uint32_t size, const eMemoryFlag ramflag, void *param);
typedef void *(*funcMemRealloc)(void *pPtr, const size_t newSize, void *param);
typedef void (*funcMemFree)(void* ptr, void *param);

typedef struct SUSER_MEMORY_CALLBACKS
{
  funcMemAlloc cbUserMemAlloc;
  funcMemFree cbUserMemFree;
  funcMemRealloc cbMemRealloc;
} sUserMemoryCallbacks;

AM_EXTERN funcMemAlloc gUserMemAlloc;
AM_EXTERN funcMemFree gUserMemFree;
AM_EXTERN funcMemRealloc gUserMemRealloc;

void *amMalloc(const uint32_t size, const eMemoryFlag ramflag, void *param);
void amFree(void *ptr, void *param);

static inline int amMemCmp ( const void *pSrc1, const void *pSrc2, const MemSize iNum)
{
  return memcmp(pSrc1,pSrc2,iNum);
}

static inline void *amMemMove (void *pDest, const void *pSrc, const MemSize iSize)
{
 return memmove(pDest,pSrc,iSize);
}

static inline void *amMemCpy (void *pDest, const void *pSrc, const MemSize iSize)
{
  uint8 *pbDest=(uint8 *)pDest;
  uint8 *pbSrc=(uint8 *)pSrc;

  if( (pbSrc<pbDest && (pbSrc + iSize)>pbDest ) || (pbDest<pbSrc && (pbDest +iSize) >pbSrc))
  {
    #ifdef DEBUG_MEM
      amTrace("\tamMemCpy() overlaps. Using amMemMove()" NL,0);
    #endif
    return amMemMove(pDest,pSrc,iSize);
  }

  return memcpy(pDest,pSrc,iSize);
}

static inline void *amMemSet ( void *pSrc, const int32 iCharacter, const MemSize iNum)
{
  void *pPtr=0;
  pPtr = memset(pSrc,iCharacter,iNum);

  #ifdef DEBUG_MEM
    if(pPtr!=pSrc) amTrace((const uint8 *)"\tamMemSet() warning: returned pointers aren't equal!"NL);
    else{
      amTrace("\tamMemSet() memory: %p, %d value written: %x!" NL,pSrc,iNum,iCharacter);
    }
  #endif

  return pPtr;
}

// sets user memory allocation functions
void amSetDefaultUserMemoryCallbacks(void);

// sets user memory allocation functions
void amSetUserMemoryCallbacks(sUserMemoryCallbacks *func);

/**
 * gets amount of free preferred memory type (ST/FastRam/SuperVidel).
 * @param memFlag memory allocation preference flag
 * @return 0L - if no memory available, 0L< otherwise
 */

uint32 amGetFreeMemory(const eMemoryFlag memFlag); 

#endif

