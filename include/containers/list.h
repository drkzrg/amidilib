
/**  Copyright 2007-2021 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/


#ifndef __LIST_H__
#define __LIST_H__

#include "../amidiseq.h"

#ifdef EVENT_LINEAR_BUFFER
void initEventList(sSequence_t *pSequence, sEventList **listPtr);
int16 addEvent(sSequence_t *pSequence, sEventList **listPtr, const sEventBlock_t *eventBlockPtr );
int16 copyEvent(sSequence_t *pSequence, const sEventBlock_t *src, sEventList **dest);
uint32 destroyList(sSequence_t *pSequence, sEventList **listPtr);
#else
void initEventList(sEventList **listPtr);
int16 addEvent(sEventList **listPtr, const sEventBlock_t *eventBlockPtr );
int16 copyEvent(const sEventBlock_t *src, sEventList **dest);
uint32 destroyList(sEventList **listPtr);
#endif

/* for debugging purposes */
void printEventList(const sEventList *listPtr);
void printEventBlock(const sEventBlockPtr_t pPtr);

#endif
