
/**  Copyright 2007-2010 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#ifndef __AMEMORY_TOS_H__
#define __AMEMORY_TOS_H__


#include <mint/ostruct.h>
#include <mint/osbind.h>
#include "amlog.h"
#include "c_vars.h"

#include <string.h>
#include <stdlib.h>

/* memory allocation preference */

typedef enum {
  ST_RAM = MX_STRAM,
  TT_RAM = MX_TTRAM,
  PREFER_ST = MX_PREFSTRAM,
  PREFER_TT = MX_PREFTTRAM,
  PREFER_DSP = PREFER_TT+1,	   //f030 only, not used atm
  PREFER_SUPERVIDEL = PREFER_TT+2, //f030 only, not used atm
  PREFER_RADEON = PREFER_TT+3	   //f030 only, not used atm
} eMemoryFlag;

static inline void *amMallocEx(const tMEMSIZE amount,const  U16 flag){
void *pMem=0;

#if defined (FORCE_MALLOC)
  pMem = malloc(amount);
#else
  //TODO: check gemdos version and use Malloc() if needed for plain ST/e compatibility
  // make word alignment

  //TODO: add memory alignment build option on word, long boundary

  pMem= (void *)Mxalloc(amount,flag);
#endif

  #ifdef DEBUG_MEM
    if(pMem==0) {
      amTrace((const U8 *)"\tamMallocEx() Memory allocation error, returned NULL pointer! memory flag: %x\n",flag);
    }else {
      g_memAllocTT++;
      amTrace((const U8 *)"\tamMallocEx() allocated %ld bytes at %p, mem flag: %x\n[AltAlloc nb: %d][memory left: %d]\n",amount,pMem,flag, g_memAllocTT, getFreeMem(flag) );
    }
  #endif

  return (void *)pMem;
}

static inline void *amMalloc(const tMEMSIZE amount){
void *pMem=0;

//TODO: add memory alignment build option on word, long boundary

#if defined (FORCE_MALLOC)
  pMem= malloc(amount);
#else
  pMem=(void *)Malloc(amount);
#endif

  #ifdef DEBUG_MEM
    if(pMem==0) {
      amTrace((const U8 *)"\tamMalloc() Memory allocation error,\n returned NULL pointer!!!!!\n");
    }
    else {
      g_memAlloc++;
      amTrace((const U8 *)"\tamMalloc() allocated %ld bytes at %p.\n[alloc nb: %d][memory left: %d]\n",amount,pMem,g_memAlloc,getFreeMem(PREFER_ST));
    }
  #endif
 return pMem;
}

static inline void amFree(void **pPtr){

  #ifdef DEBUG_MEM
    if(*pPtr==0) {
      amTrace((const U8 *)"\tamFree() WARING: NULL pointer passed. \n");
    }
    else {
      g_memDealloc++;
      amTrace((const U8 *)"\tamFree() releasing memory at at %p [dealloc: %d]\n",*pPtr,g_memDealloc);
    }
  #endif

 #if defined (FORCE_MALLOC)
  free(*pPtr); *pPtr=0;
 #else
    Mfree(*pPtr); *pPtr=0;
 #endif
}

static inline int amMemCmp ( void *pSrc1, void *pSrc2, const tMEMSIZE iNum){
  return memcmp(pSrc1,pSrc2,iNum);
}

static inline void *amMemMove (void *pDest,void *pSrc,const tMEMSIZE iSize){
 return memmove(pDest,pSrc,iSize);
}


static inline void *amMemCpy (void *pDest, void *pSrc,const tMEMSIZE iSize){
  U8 *pbDest=(U8 *)pDest;
  U8 *pbSrc=(U8 *)pSrc;

  if( (pbSrc<pbDest && (pbSrc + iSize)>pbDest ) || (pbDest<pbSrc && (pbDest +iSize) >pbSrc)){
    #ifdef DEBUG_MEM
      amTrace((const U8 *)"\tamMemCpy() overlaps. Using amMemMove()\n");
    #endif
    return amMemMove(pDest,pSrc,iSize);
  }

  return memcpy(pDest,pSrc,iSize);
}

static inline void *amMemSet ( void *pSrc,const S32 iCharacter,const tMEMSIZE iNum){
void *pPtr=0;

  pPtr= memset(pSrc,iCharacter,iNum);

  #ifdef DEBUG_MEM
    if(pPtr!=pSrc) amTrace((const U8 *)"\tamMemSet() warning: returned pointers aren't equal!\n");
    else{
      amTrace((const U8 *)"\tamMemSet() memory: %p, %d x value written: %x!\n",pSrc,iNum,iCharacter);
    }
  #endif

  return pPtr;
}

static inline void *amMemChr ( void *pSrc,const S32 iCharacter,const tMEMSIZE iNum){
  return memchr(pSrc,iCharacter,iNum);
}


static inline void *amCalloc(const tMEMSIZE nelements,const tMEMSIZE elementSize){
  return calloc(nelements,elementSize);
}

static inline void *amRealloc( void *pPtr, const tMEMSIZE newSize){
 return realloc(pPtr,newSize);
}

/**
 * gets amount of free preferred memory type (ST/TT RAM).
 * @param memFlag memory allocation preference flag
 * @return 0L - if no memory available, 0L< otherwise
 */
U32 getFreeMem(const eMemoryFlag memFlag);


#ifdef DEBUG_BUILD
/** function logs amount of all types of memory present in the system   
 */
static U32 g_memAlloc=0;
static U32 g_memAllocTT=0;
static U32 g_memDealloc=0;

void memoryCheck(void){
    U32 mem=0;
    amTrace((const U8*)"System memory check:\n");

    /* mem tst */
    mem=getFreeMem(ST_RAM);
    amTrace((const U8*)"ST-RAM: %u\n",(U32)mem);

    mem=getFreeMem(TT_RAM);
    amTrace((const U8*)"TT-RAM: %u\n",(U32)mem);

    mem=getFreeMem(PREFER_ST);
    amTrace((const U8*)"Prefered ST-RAM: %u\n",(U32)mem);

    mem=getFreeMem(PREFER_TT);
    amTrace((const U8*)"Prefered TT-RAM: %u\n",(U32)mem);
}

#endif

// helpers writes a byte/short/long and returns the buffer
U8* WriteByte(void* buf, S8 b);
U8* WriteShort(void* b, U16 s);
U8* WriteInt(void* b, U32 i);
S32 UpdateBytesWritten(S32* bytes_written, S32 to_add, S32 max);

#endif

