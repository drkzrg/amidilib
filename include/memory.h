
/**  Copyright 2007-2010 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#ifndef __AMEMORY_H__
#define __AMEMORY_H__

#ifdef PORTABLE
#include <stdlib.h>

/* memory allocation preference */
// compatible with Mxalloc
typedef enum
{
  ST_RAM=0,
  TT_RAM=0,
  PREFER_ST=0,
  PREFER_TT=0
} eMemoryFlag;


#else
#include <stdlib.h>
#include <mint/osbind.h>
#include <mint/ostruct.h>

/* memory allocation preference */
// compatible with Mxalloc
typedef enum
{
  ST_RAM=MX_STRAM,
  TT_RAM=MX_TTRAM,
  PREFER_ST=MX_PREFSTRAM,
  PREFER_TT=MX_PREFTTRAM
} eMemoryFlag;

#endif

#include "c_vars.h"
#include "amlog.h"

/**
 * gets amount of free preferred memory type (ST/TT RAM).
 * @param memFlag memory allocation preference flag
 * @return 0L - if no memory available, 0L< otherwise
 */
 inline U32 getFreeMem(eMemoryFlag memFlag){
void *pMem=0;
#ifdef PORTABLE
  //TODO:
   pMem=(void *)~0L;
#else
  pMem=(void *)Mxalloc( -1L, memFlag);
#endif  
    return((U32)pMem);
}



inline void *amMemMove (void *pDest,void *pSrc,tMEMSIZE iSize){
#ifdef PORTABLE
  return memmove(pDest,pSrc,iSize);
#else
 return memmove(pDest,pSrc,iSize);
#endif
}


 inline void *amMemCpy (void *pDest, void *pSrc,tMEMSIZE iSize){
#ifdef PORTABLE
  U8 *pbDest=(U8 *)pDest;
  U8 *pbSrc=(U8 *)pSrc;
 
  if( (pbSrc<pbDest && (pbSrc + iSize)>pbDest ) || (pbDest<pbSrc && (pbDest +iSize) >pbSrc)){
 
     #ifdef DEBUG_BUILD
      amTrace((const U8 *)"\tamMemCpy() overlaps. Using amMemMove()\n");
     #endif
 
    return amMemMove(pDest,pSrc,iSize);
  }
  
  return memcpy(pDest,pSrc,iSize);
#else
  U8 *pbDest=(U8 *)pDest;
  U8 *pbSrc=(U8 *)pSrc;
  
  if( (pbSrc<pbDest && (pbSrc + iSize)>pbDest ) || (pbDest<pbSrc && (pbDest +iSize) >pbSrc)){
    #ifdef DEBUG_MEM
      amTrace((const U8 *)"\tamMemCpy() overlaps. Using amMemMove()\n");
    #endif    
    return amMemMove(pDest,pSrc,iSize);
  }
  
  return memcpy(pDest,pSrc,iSize);
#endif
}

  inline void *amMemSet ( void *pSrc,S32 iCharacter,tMEMSIZE iNum)
{
  void *pPtr=0;
  
#ifdef PORTABLE
  pPtr = memset(pSrc,iCharacter,iNum);
#else
  pPtr= memset(pSrc,iCharacter,iNum);
#endif
  
  #ifdef DEBUG_MEM
    if(pPtr!=pSrc) amTrace((const U8 *)"\tamMemSet() warning: returned pointers aren't equal!\n");
    else{
      amTrace((const U8 *)"\tamMemSet() memory: %p, %d x value written: %x!\n",pSrc,iNum,iCharacter);
    }
  #endif
  
  return pPtr;
}

  inline int amMemCmp ( void *pSrc1, void *pSrc2, tMEMSIZE iNum){
#ifdef PORTABLE
  return memcmp(pSrc1,pSrc2,iNum);
#else
  return memcmp(pSrc1,pSrc2,iNum);
#endif
}

  inline void *amMemChr ( void *pSrc, S32 iCharacter, tMEMSIZE iNum){
#ifdef PORTABLE
  return memchr(pSrc,iCharacter,iNum);
#else
  return memchr(pSrc,iCharacter,iNum);
#endif
}

 inline void *amMallocEx(tMEMSIZE amount, U16 flag){
void *pMem=0;

#ifdef PORTABLE
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
      amTrace((const U8 *)"\tamMallocEx() allocated %ld bytes at %p, mem flag: %x\n",amount,pMem, flag);
    }
  #endif
  
  return (void *)pMem;
  
}

 static inline void *amMalloc(tMEMSIZE amount){
void *pMem=0;
//TODO: add memory alignment build option on word, long boundary

#ifdef PORTABLE
  pMem= malloc(amount); 
#else
  pMem=(void *)Malloc(amount);
#endif
  
  #ifdef DEBUG_MEM
    if(pMem==0) amTrace((const U8 *)"\tamMalloc() Memory allocation error,\n returned NULL pointer!!!!!\n");
    else {
      amTrace((const U8 *)"\tamMalloc() allocated %ld bytes at %p\n",amount,pMem);
    }
  #endif
 return pMem;
}

 static inline void amFree(void *pPtr){

  #ifdef DEBUG_MEM
    if(pPtr==0) {
      amTrace((const U8 *)"\tamFree() WARING: NULL pointer passed. \n");
    }
    else {
      amTrace((const U8 *)"\tamFree() releasing memory at at %p\n",pPtr);
    }
  #endif
  
  #ifdef PORTABLE
    free(pPtr); pPtr=0;
  #else
    Mfree(pPtr); pPtr=0;
  #endif
}

 inline void *amCalloc(tMEMSIZE nelements, tMEMSIZE elementSize){
#ifdef PORTABLE
  return calloc(nelements,elementSize);
#else
  return calloc(nelements,elementSize);
#endif
}


 inline void *amRealloc( void *pPtr, tMEMSIZE newSize){
#ifdef PORTABLE
 return realloc(pPtr,newSize);
#else
 return realloc(pPtr,newSize);
#endif
}

#endif

