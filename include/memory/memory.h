
/**  Copyright 2007-2021 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#ifndef __AMEMORY_TOS_H__
#define __AMEMORY_TOS_H__

#include "vartypes.h"
#include "amlog.h"

#if AMIDILIB_USE_LIBC
#include <string.h>
#else
#include "amstring.h"
#endif

/* memory allocation preference */

#if defined (FORCE_MALLOC)

typedef enum EMEMORYFLAG 
{
  ST_RAM = 0,
  TT_RAM ,
  PREFER_ST,
  PREFER_TT,
  PREFER_DSP,         
  PREFER_SUPERVIDEL, 
} eMemoryFlag;

#else

#include <mint/ostruct.h>

typedef enum EMEMORYFLAG 
{
  ST_RAM = MX_STRAM,
  TT_RAM = MX_TTRAM,
  PREFER_ST = MX_PREFSTRAM,
  PREFER_TT = MX_PREFTTRAM,
  PREFER_DSP = PREFER_TT + 1,         // f030 only, not used atm
  PREFER_SUPERVIDEL = PREFER_TT + 2,  // f030 + CT60 only, not used atm
} eMemoryFlag;

#endif

/* memory callback types */
typedef void *(*funcMemAlloc)(size_t size, uint32 memflag, void *param);
typedef void *(*funcMemRealloc)(void *pPtr, const size_t newSize, void *param);
typedef void (*funcMemFree)(void* ptr, void *param);

typedef struct SUSER_MEMORY_CALLBACKS
{
  funcMemAlloc cbUserMemAlloc;
  funcMemFree cbUserMemFree;
  funcMemRealloc cbMemRealloc;
} sUserMemoryCallbacks;

extern funcMemAlloc gUserMemAlloc;
extern funcMemFree gUserMemFree;
extern funcMemRealloc gUserMemRealloc;

void *amMalloc(unsigned int size, short int ramflag, void *param);
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
      amTrace((const uint8 *)"\tamMemCpy() overlaps. Using amMemMove()\n");
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
    if(pPtr!=pSrc) amTrace((const uint8 *)"\tamMemSet() warning: returned pointers aren't equal!\n");
    else{
      amTrace((const uint8 *)"\tamMemSet() memory: %p, %d value written: %x!\n",pSrc,iNum,iCharacter);
    }
  #endif

  return pPtr;
}

// sets user memory allocation functions
void amSetDefaultUserMemoryCallbacks(void);

// sets user memory allocation functions
void amSetUserMemoryCallbacks(sUserMemoryCallbacks *func);

// helpers writes a byte/short/long and returns the buffer
uint8* WriteByte(void* buf, int8 b);
uint8* WriteShort(void* b, uint16 s);
uint8* WriteInt(void* b, uint32 i);
int32 UpdateBytesWritten(int32* bytes_written, int32 to_add, int32 max);

#endif

