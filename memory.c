
/**  Copyright 2007-2010 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#include "include/memory.h"

extern void *amMallocEx (tMEMSIZE amount, U16 flag);
extern void *amMalloc (tMEMSIZE amount);
extern void amFree (void *pPtr);
extern void *amMemCpy (void *pDest,const void *pSrc,tMEMSIZE iSize);
extern void *amMemSet (void *pSrc,S32 iCharacter,tMEMSIZE iNum);
extern int amMemCmp (const void *pSrc1,const void *pSrc2, tMEMSIZE iNum);
extern void *amMemChr (const void *pSrc, S32 iCharacter, tMEMSIZE iNum);
extern void *amMemMove (void *pDest,const void *pSrc,tMEMSIZE iSize);
extern void *amCalloc (tMEMSIZE nelements, tMEMSIZE elementSize);
extern void *amRealloc(void *pPtr, tMEMSIZE newSize);
