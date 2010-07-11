
/**  Copyright 2007-2010 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/


#ifndef __LIST_H__
#define __LIST_H__

#include "../amidiseq.h"


void initEventList(sEventList *listPtr);
U32 addEvent(sEventList *listPtr, sEventBlock_t *eventBlockPtr );
U32 destroyList(sEventList *listPtr);

/* for debugging purposes */
void printEventList(const sEventList **listPtr);
void printEventBlock(U32 counter,volatile sEventBlockPtr_t pPtr);

U32 sendMidiEvents(U32 delta_start, const sEventList **listPtr);



#endif
