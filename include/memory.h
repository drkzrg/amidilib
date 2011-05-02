
/**  Copyright 2007-2010 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#ifndef __AMEMORY_H__
#define __AMEMORY_H__

#include "c_vars.h"

#ifdef PORTABLE
#include <stdlib.h>
#else
#include <mint/sysbind.h>
#endif


inline void *amMemCpy (void *pDest,const void *pSrc,tMEMSIZE iSize){
#ifdef PORTABLE

#else

#endif
 return 0;  
}

inline void *amMemSet (void *pSrc,int iCharacter,tMEMSIZE iNum)
{
#ifdef PORTABLE

#else

#endif

  return 0;
}

inline int amMemCmp (const void *pSrc1,const void *pSrc2, tMEMSIZE iNum){
#ifdef PORTABLE

#else

#endif

  return 0;
}

inline void *amMemChr (const void *pSrc, int iCharacter, tMEMSIZE iNum){
#ifdef PORTABLE

#else

#endif
  return 0;
}

inline void *amMemMove (void *pDest,const void *pSrc,tMEMSIZE iSize){
#ifdef PORTABLE

#else

#endif

  
return 0;
}

inline void *amMalloc(tMEMSIZE amount){
  
#ifdef PORTABLE

#else

#endif
  
  return 0;
  
}

inline void amFree(void *pPtr){

#ifdef PORTABLE

#else

#endif
}

inline void *amCalloc(tMEMSIZE nelements, tMEMSIZE elementSize){
#ifdef PORTABLE

#else

#endif
  return 0;
}


inline void *amRealloc(void *pPtr, tMEMSIZE newSize){
#ifdef PORTABLE

#else

#endif
  return 0;
}

#endif
