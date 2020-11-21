
/**  Copyright 2007-2020 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/


#ifndef __LIST_H__
#define __LIST_H__

#include "../amidiseq.h"

#ifdef EVENT_LINEAR_BUFFER
void initEventList(sSequence_t *pSequence, sEventList **listPtr);
retVal addEvent(sSequence_t *pSequence, sEventList **listPtr, const sEventBlock_t *eventBlockPtr );
retVal copyEvent(sSequence_t *pSequence, const sEventBlock_t *src, sEventList **dest);
uint32 destroyList(sSequence_t *pSequence, sEventList **listPtr);
#else
void initEventList(sEventList **listPtr);
retVal addEvent(sEventList **listPtr, const sEventBlock_t *eventBlockPtr );
retVal copyEvent(const sEventBlock_t *src, sEventList **dest);
uint32 destroyList(sEventList **listPtr);
#endif

/* for debugging purposes */
void printEventList(const sEventList *listPtr);
void printEventBlock(const sEventBlockPtr_t pPtr);

#endif
