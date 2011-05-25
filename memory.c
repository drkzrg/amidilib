
/**  Copyright 2007-2010 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#include "include/memory.h"

#ifdef DEBUG_MEM
static U32 g_memAlloc=0;
static U32 g_memAllocTT=0;
static U32 g_memDealloc=0;
#endif

/**
 * gets amount of free preferred memory type (ST/TT RAM).
 * @param memFlag memory allocation preference flag
 * @return 0L - if no memory available, 0L< otherwise
 */
U32 getFreeMem(eMemoryFlag memFlag){
void *pMem=0;
#ifdef PORTABLE
  //TODO:
   pMem=(void *)~0L;
#else
  pMem=(void *)Mxalloc( -1L, memFlag);
#endif  
    return((U32)pMem);
}



void *amMemMove (void *pDest,void *pSrc,tMEMSIZE iSize){
#ifdef PORTABLE
  return memmove(pDest,pSrc,iSize);
#else
 return memmove(pDest,pSrc,iSize);
#endif
}


   void *amMemCpy (void *pDest, void *pSrc,tMEMSIZE iSize){
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

void *amMemSet ( void *pSrc,S32 iCharacter,tMEMSIZE iNum){
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

int amMemCmp ( void *pSrc1, void *pSrc2, tMEMSIZE iNum){
#ifdef PORTABLE
  return memcmp(pSrc1,pSrc2,iNum);
#else
  return memcmp(pSrc1,pSrc2,iNum);
#endif
}

void *amMemChr ( void *pSrc, S32 iCharacter, tMEMSIZE iNum){
#ifdef PORTABLE
  return memchr(pSrc,iCharacter,iNum);
#else
  return memchr(pSrc,iCharacter,iNum);
#endif
}

void *amMallocEx(tMEMSIZE amount, U16 flag){
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
      g_memAllocTT++;
      amTrace((const U8 *)"\tamMallocEx() allocated %ld bytes at %p, mem flag: %x\n[AltAlloc nb: %d][memory left: %d]\n",amount,pMem,flag, g_memAllocTT, getFreeMem(flag) );
    }
  #endif
  
  return (void *)pMem;
}

void *amMalloc(tMEMSIZE amount){
void *pMem=0;

//TODO: add memory alignment build option on word, long boundary

#ifdef PORTABLE
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

void amFree(void **pPtr){

  #ifdef DEBUG_MEM
    if(*pPtr==0) {
      amTrace((const U8 *)"\tamFree() WARING: NULL pointer passed. \n");
    }
    else {
      g_memDealloc++;
      amTrace((const U8 *)"\tamFree() releasing memory at at %p [dealloc: %d]\n",*pPtr,g_memDealloc);
    }
  #endif
  
  #ifdef PORTABLE
    free(*pPtr); *pPtr=0;
  #else
    Mfree(*pPtr); *pPtr=0;
  #endif
}

void *amCalloc(tMEMSIZE nelements, tMEMSIZE elementSize){
#ifdef PORTABLE
  return calloc(nelements,elementSize);
#else
  return calloc(nelements,elementSize);
#endif
}


void *amRealloc( void *pPtr, tMEMSIZE newSize){
#ifdef PORTABLE
 return realloc(pPtr,newSize);
#else
 return realloc(pPtr,newSize);
#endif
}


