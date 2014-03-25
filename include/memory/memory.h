
/**  Copyright 2007-2010 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#ifndef __AMEMORY_H__
#define __AMEMORY_H__

#include "c_vars.h"

#include <alloca.h> //todo: remove alloca from code

#include <mint/osbind.h>
#include <mint/ostruct.h>

/* memory allocation preference */

typedef enum{
  ST_RAM = MX_STRAM,
  TT_RAM = MX_TTRAM,
  PREFER_ST = MX_PREFSTRAM,
  PREFER_TT = MX_PREFTTRAM,
  PREFER_DSP = PREFER_TT+1,	   //f030 only, not used atm
  PREFER_SUPERVIDEL = PREFER_TT+2, //f030 only, not used atm
  PREFER_RADEON = PREFER_TT+3	   //f030 only, not used atm
} eMemoryFlag;

void *amMallocEx (const tMEMSIZE amount,const U16 flag);
void *amMalloc (const tMEMSIZE amount);

void amFree (void **pPtr);
int amMemCmp ( void *pSrc1, void *pSrc2, const tMEMSIZE iNum);
void *amMemCpy ( void *pDest, void *pSrc,const tMEMSIZE iSize);
void *amMemSet ( void *pSrc,const S32 iCharacter,const tMEMSIZE iNum);
void *amMemChr ( void *pSrc,const S32 iCharacter,const tMEMSIZE iNum);
void *amMemMove ( void *pDest, void *pSrc,const tMEMSIZE iSize);
void *amCalloc (const tMEMSIZE nelements,const tMEMSIZE elementSize);
void *amRealloc(void *pPtr,const tMEMSIZE newSize);
U32 getFreeMem(const eMemoryFlag memFlag);

#ifdef DEBUG_BUILD
/** function logs amount of all types of memory present in the system   
 */
void memoryCheck(void);
#endif

// helpers writes a byte/short/long and returns the buffer
U8* WriteByte(void* buf, S8 b);
U8* WriteShort(void* b, U16 s);
U8* WriteInt(void* b, U32 i);
S32 UpdateBytesWritten(S32* bytes_written, S32 to_add, S32 max);


#endif

