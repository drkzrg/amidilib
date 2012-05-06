
/**  Copyright 2007-2010 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#ifndef __AMEMORY_H__
#define __AMEMORY_H__

#include <stdlib.h>
#include <alloca.h>

#ifdef PORTABLE
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

void *amMallocEx (tMEMSIZE amount, U16 flag);
void *amMalloc (tMEMSIZE amount);
void *amAlloca(tMEMSIZE amount);

void amFree (void **pPtr);
void *amMemCpy ( void *pDest, const void *pSrc,tMEMSIZE iSize);
void *amMemSet ( void *pSrc,S32 iCharacter,tMEMSIZE iNum);
int amMemCmp ( const void *pSrc1, const void *pSrc2, tMEMSIZE iNum);
void *amMemChr ( void *pSrc, S32 iCharacter, tMEMSIZE iNum);
void *amMemMove ( void *pDest, const void *pSrc,tMEMSIZE iSize);
void *amCalloc (tMEMSIZE nelements, tMEMSIZE elementSize);
void *amRealloc( const void *pPtr, tMEMSIZE newSize);
U32 getFreeMem(eMemoryFlag memFlag);

#ifdef DEBUG_BUILD
/** function logs amount of all types of memory present in the system   
 */
void memoryCheck(void);
#endif

#endif

