
/**  Copyright 2007-2010 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#ifndef __AMEMORY_H__
#define __AMEMORY_H__

#include "c_vars.h"
#include "amlog.h"

#ifdef PORTABLE
#include <stdlib.h>
#else
#include <stdlib.h>
#include <mint/sysbind.h>
#include <mint/ostruct.h>
#endif

inline void *amMemMove (void *pDest,const void *pSrc,tMEMSIZE iSize){
#ifdef PORTABLE
  return memmove(pDest,pSrc,iSize);
#else
 return memmove(pDest,pSrc,iSize);
#endif
}


inline void *amMemCpy (void *pDest,const void *pSrc,tMEMSIZE iSize){
#ifdef PORTABLE
  U8 *pbDest=(U8 *)pDest;
  U8 *pbSrc=(U8 *)pSrc;

  amTrace((const U8 *)"amMemCpy overlaps. Using amMemMove()\n");
  
  if( (pbSrc<pbDest && pbSrc + iSize>pbDest ) ||
    (pbDest<pbSrc && pbDest +iSize >pbSrc)){
    
    return amMemMove(pDest,pSrc,iSize);
  }
  
  return memcpy(pDest,pSrc,iSize);
#else
  U8 *pbDest=(U8 *)pDest;
  U8 *pbSrc=(U8 *)pSrc;
  
  if( (pbSrc<pbDest && pbSrc + iSize>pbDest ) ||
    (pbDest<pbSrc && pbDest +iSize >pbSrc)){

    amTrace((const U8 *)"amMemCpy overlaps. Using amMemMove()\n");
    return amMemMove(pDest,pSrc,iSize);
  }
  return memcpy(pDest,pSrc,iSize);
#endif
}

inline void *amMemSet (void *pSrc,S32 iCharacter,tMEMSIZE iNum)
{
#ifdef PORTABLE
  return memset(pSrc,iCharacter,iNum);
#else
  return memset(pSrc,iCharacter,iNum);
#endif
}

inline int amMemCmp (const void *pSrc1,const void *pSrc2, tMEMSIZE iNum){
#ifdef PORTABLE
  return memcmp(pSrc1,pSrc2,iNum);
#else
  return memcmp(pSrc1,pSrc2,iNum);
#endif
}

inline void *amMemChr (const void *pSrc, S32 iCharacter, tMEMSIZE iNum){
#ifdef PORTABLE
  return memchr(pSrc,iCharacter,iNum);
#else
  return memchr(pSrc,iCharacter,iNum);
#endif
}



inline void *amMallocEx(tMEMSIZE amount, U16 flag){
#ifdef PORTABLE
  return malloc(amount);
#else
  return Mxalloc(amount,flag);
#endif
}

inline void *amMalloc(tMEMSIZE amount){
#ifdef PORTABLE
  return malloc(amount);
#else
  //prefer TT-RAM by default
  return Mxalloc(amount,MX_PREFTTRAM);
#endif
}



inline void amFree(void *pPtr){

#ifdef PORTABLE
  free(pPtr); pPtr=0;
#else
  Mfree(pPtr);
#endif
}

inline void *amCalloc(tMEMSIZE nelements, tMEMSIZE elementSize){
#ifdef PORTABLE
  return calloc(nelements,elementSize);
#else
  return calloc(nelements,elementSize);
#endif
}


inline void *amRealloc(void *pPtr, tMEMSIZE newSize){
#ifdef PORTABLE
 return realloc(pPtr,newSize);
#else
 return realloc(pPtr,newSize);
#endif
}

#endif
