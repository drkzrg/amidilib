
/**  Copyright 2007-2010 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#include "include/memory.h"

extern inline void *amMallocEx (tMEMSIZE amount, U16 flag);
extern inline void *amMalloc (tMEMSIZE amount);
extern inline void amFree (void *pPtr);
extern inline void *amMemCpy ( void *pDest, void *pSrc,tMEMSIZE iSize);
extern inline void *amMemSet ( void *pSrc,S32 iCharacter,tMEMSIZE iNum);
extern inline int amMemCmp ( void *pSrc1, void *pSrc2, tMEMSIZE iNum);
extern inline void *amMemChr ( void *pSrc, S32 iCharacter, tMEMSIZE iNum);
extern inline void *amMemMove ( void *pDest, void *pSrc,tMEMSIZE iSize);
extern inline void *amCalloc (tMEMSIZE nelements, tMEMSIZE elementSize);
extern inline void *amRealloc( void *pPtr, tMEMSIZE newSize);
extern inline U32 getFreeMem(eMemoryFlag memFlag);
