
/**  Copyright 2007-2020 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#ifndef __AMEMORY_TOS_H__
#define __AMEMORY_TOS_H__

#include "c_vars.h"
#include "amlog.h"

#if !defined(FORCE_MALLOC)
#include <mint/ostruct.h>
#include <mint/osbind.h>
#endif

#include <stdlib.h>
#include <string.h>

/* memory allocation preference */
typedef enum EMEMORYFLAG 
{
  ST_RAM = MX_STRAM,
  TT_RAM = MX_TTRAM,
  PREFER_ST = MX_PREFSTRAM,
  PREFER_TT = MX_PREFTTRAM,
  PREFER_DSP = PREFER_TT + 1,         // f030 only, not used atm
  PREFER_SUPERVIDEL = PREFER_TT + 2,  // f030 + CT60 only, not used atm
} eMemoryFlag;

#if defined (EXTERN_MEM_FUNCTIONS)

extern void  amUserFree (void* ptr, void *param);
extern void* amUserMalloc (unsigned int size, void *param);
extern void* amUserMallocEx (unsigned int size, short int ramflag, void *param);

#define amMallocEx(amount, flag) amUserMallocEx((uint32)amount, (uint16)flag, NULL);
#define amMalloc(amount) amUserMalloc((uint32)amount, NULL);
#define amFree(memPtr) AssertMsg(memPtr!=NULL,"amUserFree() called on NULL address!"); amUserFree(memPtr,NULL); memPtr = NULL;

#else

#if defined (FORCE_MALLOC)

#define amMallocEx(amount, flag) malloc((uint32)amount);
#define amMalloc(amount) malloc((uint32)amount);
#define amFree(memPtr) AssertMsg(memPtr!=NULL,"free() called on NULL address!"); free(memPtr); memPtr = NULL;

#else

#if defined(TARGET_ST)
#define amMallocEx(amount, flag) (void *)Malloc((uint32)amount);
#define amMalloc(amount) (void *)Malloc((uint32)amount);
#else
#define amMallocEx(amount, flag) (void *)Mxalloc((uint32)amount,(int16)flag);
#define amMalloc(amount) (void *)Malloc((uint32)amount);

#endif

#define amFree(memPtr) AssertMsg(memPtr!=NULL,"MFree() called on NULL address!"); Mfree(memPtr); memPtr = NULL;

#endif // not extern mem functions

#endif

static inline int amMemCmp ( const void *pSrc1, const void *pSrc2, const MemSize iNum){
  return memcmp(pSrc1,pSrc2,iNum);
}

static inline void *amMemMove (void *pDest, const void *pSrc, const MemSize iSize){
 return memmove(pDest,pSrc,iSize);
}


static inline void *amMemCpy (void *pDest, const void *pSrc, const MemSize iSize){
  uint8 *pbDest=(uint8 *)pDest;
  uint8 *pbSrc=(uint8 *)pSrc;

  if( (pbSrc<pbDest && (pbSrc + iSize)>pbDest ) || (pbDest<pbSrc && (pbDest +iSize) >pbSrc)){
    #ifdef DEBUG_MEM
      amTrace((const uint8 *)"\tamMemCpy() overlaps. Using amMemMove()\n");
    #endif
    return amMemMove(pDest,pSrc,iSize);
  }

  return memcpy(pDest,pSrc,iSize);
}

static inline void *amMemSet ( void *pSrc, const int32 iCharacter, const MemSize iNum){
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

static inline void *amMemChr ( void *pSrc, const int32 iCharacter, const MemSize iNum){
  return memchr(pSrc,iCharacter,iNum);
}


static inline void *amCalloc(const MemSize nelements, const MemSize elementSize){
  return calloc(nelements,elementSize);
}

static inline void *amRealloc( void *pPtr, const MemSize newSize){
 return realloc(pPtr,newSize);
}



/**
 * gets amount of free preferred memory type (ST/TT RAM).
 * @param memFlag memory allocation preference flag
 * @return 0L - if no memory available, 0L< otherwise
 */
uint32 getFreeMem(const eMemoryFlag memFlag);


#ifdef DEBUG_BUILD
/** function logs amount of all types of memory present in the system   
 */

void memoryCheck(void);

#endif

// helpers writes a byte/short/long and returns the buffer
uint8* WriteByte(void* buf, int8 b);
uint8* WriteShort(void* b, uint16 s);
uint8* WriteInt(void* b, uint32 i);
int32 UpdateBytesWritten(int32* bytes_written, int32 to_add, int32 max);

#endif

