
#ifndef __MIDISEQ_H__
#define __MIDISEQ_H__

/**  Copyright 2007-2010 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#include "c_vars.h"
#include "midi.h"


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
U8 ubChannelNb;			/* channel number */
sNoteOn_t eventData;	/*note on data */
} __attribute__((__packed__)) sNoteOn_EventBlock_t;

/* Note Off event block */

typedef struct NoteOff_EventBlock_t{
U8 ubChannelNb;				/* channel number */
sNoteOff_t eventData;		/* note off data */
} __attribute__((__packed__)) sNoteOff_EventBlock_t;

/* Note aftertouch eventblock */
typedef struct NoteAft_EventBlock_t{
 
 U8 ubChannelNb;
 sNoteAft_t eventData;
}__attribute__((__packed__)) sNoteAft_EventBlock_t;

/* Controller change event block */
typedef struct Controller_EventBlock_t{
 U8 ubChannelNb;			 /* channel number */
 sController_t eventData;	 /* controller event data */
} __attribute__((__packed__)) sController_EventBlock_t;

/* Program change event block */
typedef struct PrgChng_EventBlock_t{
 U8 ubChannelNb;				/* channel number */
 sProgramChange_t eventData;	/* program change data */
} __attribute__((__packed__)) sPrgChng_EventBlock_t;

/* Channel aftertouch eventblock */
typedef struct ChannelAft_EventBlock_t{  
  U8 ubChannelNb;				/* channel number */
  sChannelAft_t eventData;		/* channel after touch data */
}__attribute__((__packed__)) sChannelAft_EventBlock_t;

/* Pitch bend eventblock */
typedef struct PitchBend_EventBlock_t{
 U8 ubChannelNb;			/* channel number  */
 sPitchBend_t eventData;    /* pitch bend data */
} __attribute__((__packed__)) sPitchBend_EventBlock_t;

/* Set tempo eventblock */
typedef struct Tempo_EventBlock_t{
 U8 ubChannelNb; 		/* channel number */
 sTempo_t eventData;	/* tempo event data  */
} __attribute__((__packed__)) sTempo_EventBlock_t;

/** custom type evntFuncPtr for events in given sequence  */
typedef void (*evntFuncPtr)(void *pEvent);

/** IMPORTANT! structure EventInfoBlock_t is associated with static table g_arSeqCmdTable[T_EVT_COUNT] 
    from MIDISEQ.C file changing the order of members here will affect structure of g_arSeqCmdTable[T_EVT_COUNT]. 
*/

typedef struct EventInfoBlock_t{
	U32	size;		/* size of command string in bytes */
	evntFuncPtr func;  	/* pointer to event handler */ 
} __attribute__((__packed__)) sEventInfoBlock_t;

typedef struct EventBlock_t{
 U32 uiDeltaTime;					/* event delta time */	
 sEventInfoBlock_t infoBlock;		/* function info block */ 
 void *dataPtr;						/* pointer to event data of sEventInfoBlock_t.size * 1 byte (U8) */
 U8	type;							/* event type */
 U8 pad ; 							/* padding for word alignment */
}__attribute__((__packed__)) sEventBlock_t, *sEventBlockPtr_t;

/** SysEX */
typedef struct SysEx_t{
 U32 bufferSize ; 					/* size of SysEX buffer */
 U8 *pBuffer;						/* pointer to data */
} __attribute__((__packed__)) sSysEx_t;

/* returns the info struct about event: size and pointer to the handler  */
void getEventFuncInfo(U8 eventType, sEventInfoBlock_t *infoBlk);

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

