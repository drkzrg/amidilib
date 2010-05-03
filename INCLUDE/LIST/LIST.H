
/*  Copyright 2008, 2009 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.

    AMIDILIB is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    AMIDILIB is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with AMIDILIB.  If not, see <http://www.gnu.org/licenses/>.*/


#ifndef __LIST_H__
#define __LIST_H__

#include "../AMIDISEQ.H"


void initEventList(sEventList *listPtr);
U32 addEvent(sEventList *listPtr, sEventBlock_t *eventBlockPtr );
U32 destroyList(sEventList *listPtr);

/* for debugging purposes */
void printEventList(const sEventList **listPtr);
void printEventBlock(U32 counter,const sEventBlockPtr_t *pPtr);
void playSequence(const sEventList **listPtr);
U32 sendMidiEvents(U32 delta_start, const sEventList **listPtr);

#endif
