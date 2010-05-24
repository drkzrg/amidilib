
#ifndef __MIDISEQ_H__
#define __MIDISEQ_H__

/**  Copyright 2008, 2009 Pawel Goralski
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

#include "C_VARS.H"
#include "MIDI.H"


/* max tracks per sequence */

/* event type enums */
enum eEventType{
	T_NOTEON=0,
	T_NOTEOFF,
	T_NOTEAFT,
	T_CONTROL,
	T_PRG_CH,
	T_CHAN_AFT,
	T_PITCH_BEND,
	T_META_SET_TEMPO,
	T_EVT_COUNT		/* total number of events */
};

/******************** event block structs */
/*  Note On event block */
typedef struct NoteOn_EventBlock_t{
U8 ubChannelNb;
sNoteOn_t eventData;
} sNoteOn_EventBlock_t;

/* Note Off event block */

typedef struct NoteOff_EventBlock_t{
U8 ubChannelNb;
sNoteOff_t eventData;
} sNoteOff_EventBlock_t;

/* Note aftertouch eventblock */
typedef struct NoteAft_EventBlock_t{
 
 U8 ubChannelNb;
 sNoteAft_t eventData;
}sNoteAft_EventBlock_t;

/* Controller change event block */
typedef struct Controller_EventBlock_t{
 U8 ubChannelNb;
 sController_t eventData;
} sController_EventBlock_t;

/* Program change event block */
typedef struct PrgChng_EventBlock_t{
 U8 ubChannelNb;
 sProgramChange_t eventData;
} sPrgChng_EventBlock_t;

/* Channel aftertouch eventblock */
typedef struct ChannelAft_EventBlock_t{  
  U8 ubChannelNb;
  sChannelAft_t eventData;
}sChannelAft_EventBlock_t;

/* Pitch bend eventblock */
typedef struct PitchBend_EventBlock_t{
 
 U8 ubChannelNb;
 sPitchBend_t eventData;
} sPitchBend_EventBlock_t;

/* Set tempo eventblock */
typedef struct Tempo_EventBlock_t{
 U8 ubChannelNb;
 sTempo_t eventData;
} sTempo_EventBlock_t;

/** custom type evntFuncPtr for events in given sequence  */
typedef void (*evntFuncPtr)(void *pEvent);

/** IMPORTANT! this structure is associated with static table g_arSeqCmdTable[T_EVT_COUNT] 
    from MIDISEQ.C file changing the order of members here will affect structure of g_arSeqCmdTable[T_EVT_COUNT]. 
*/

typedef struct EventInfoBlock_t{
	U32	size;			/* size of command string in bytes */
	evntFuncPtr func;  	/* pointer to event handler */ 
} sEventInfoBlock_t;

typedef struct EventBlock_t{
 U32 uiDeltaTime;			/* event delta time */
 U8	type;				/* event type */
 sEventInfoBlock_t infoBlock;		/* function info block */ 
 void *dataPtr;				/* pointer to event data of sEventInfoBlock_t.size * 1 byte (U8) */
}sEventBlock_t, *sEventBlockPtr_t;

typedef struct EventItem{
 sEventBlock_t eventBlock;
 struct EventItem *pPrev,*pNext;
}sEventItem;

/** SysEX */
typedef struct SysEx_t{
 U32 bufferSize;
 U8 *pBuffer;
} sSysEx_t;

/* returns the info struct about event: size and pointer to the handler  */
sEventInfoBlock_t getEventFuncInfo(U8 eventType);

/****************** event function prototypes */
const U8 *getEventName(U32 id);
void  fNoteOn (void *pEvent);
void  fNoteOff (void *pEvent);
void  fNoteAft (void *pEvent);
void  fProgramChange (void *pEvent);
void  fController (void *pEvent);
void  fChannelAft (void *pEvent);
void  fPitchBend (void *pEvent);
void  fSetTempo (void *pEvent);

#endif

