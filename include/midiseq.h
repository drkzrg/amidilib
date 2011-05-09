
#ifndef __MIDISEQ_H__
#define __MIDISEQ_H__

/**  Copyright 2007-2010 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#include "c_vars.h"
#include "midi.h"
#include "include/midi_cmd.h"	/* for sending midi commands */
#include "include/amlog.h" 
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


/****************** event function prototypes */
/* string table with all event names */

static const char *g_arEventNames[T_EVT_COUNT]={
	"Note On",
	"Note Off", 
	"Note Aftertouch", 
	"Controller",
	"Program Change", 
	"Channel Aftertouch", 
	"Pitch bend",
	"Set tempo(Meta)"
};



/*returns pointer to NULL terminated string with event name */
/* id is enumerated value from eEventType */
static inline const U8 *getEventName(U32 id){
	return ((const U8 *)g_arEventNames[id]);
}


static inline void  fNoteOn(void *pEvent){
	sNoteOn_EventBlock_t *pPtr=(sNoteOn_EventBlock_t *)pEvent;
	amTrace((const U8*)"Sending Note On data to sequencer ch:%d note:%d vel:%d...\n",pPtr->ubChannelNb,pPtr->eventData.noteNb,pPtr->eventData.velocity);
	
	note_on(pPtr->ubChannelNb, pPtr->eventData.noteNb,pPtr->eventData.velocity);	
}

static inline void  fNoteOff(void *pEvent){
	sNoteOff_EventBlock_t *pPtr=(sNoteOff_EventBlock_t *)pEvent;
 	amTrace((const U8*)"Sending Note Off data to sequencer ch:%d note:%d vel:%d...\n",pPtr->ubChannelNb,pPtr->eventData.noteNb,pPtr->eventData.velocity);
	
	note_off(pPtr->ubChannelNb, pPtr->eventData.noteNb,pPtr->eventData.velocity);
}

static inline void  fNoteAft(void *pEvent){
	sNoteAft_EventBlock_t *pPtr=(sNoteAft_EventBlock_t *)pEvent;	
	amTrace((const U8*)"Sending Note Aftertouch data to sequencer ch:%d note:%d pressure:%d...\n",pPtr->ubChannelNb,pPtr->eventData.noteNb,pPtr->eventData.pressure);
	
	polyphonic_key_press(pPtr->ubChannelNb,pPtr->eventData.noteNb,pPtr->eventData.pressure);
}

static inline void  fProgramChange (void *pEvent){
	sPrgChng_EventBlock_t *pPtr=(sPrgChng_EventBlock_t *)pEvent;
	amTrace((const U8*)"Sending Program change data to sequencer ch:%d pn:%d...\n",pPtr->ubChannelNb,pPtr->eventData.programNb);
	
	program_change(pPtr->ubChannelNb,pPtr->eventData.programNb);
}

static inline void  fController(void *pEvent){
	sController_EventBlock_t *pPtr=(sController_EventBlock_t *)pEvent;
	amTrace((const U8*)"Sending Controller data to sequencer ch:%d controller:%d value:%d...\n",pPtr->ubChannelNb,pPtr->eventData.controllerNb,pPtr->eventData.value);
	
	control_change(pPtr->eventData.controllerNb, pPtr->ubChannelNb, pPtr->eventData.value,0x00);
}

static inline void  fChannelAft(void *pEvent){
	sChannelAft_EventBlock_t *pPtr=(sChannelAft_EventBlock_t *)pEvent;
	amTrace((const U8*)"Sending Channel Aftertouch data to sequencer ch:%d pressure:%d...\n",pPtr->ubChannelNb,pPtr->eventData.pressure);
	
	channel_pressure (pPtr->ubChannelNb,pPtr->eventData.pressure);

}

static inline void  fPitchBend(void *pEvent){
	sPitchBend_EventBlock_t *pPtr=(sPitchBend_EventBlock_t *)pEvent;
	amTrace((const U8*)"Sending Pitch bend data to sequencer ch:%d LSB:%d MSB:%d...\n",pPtr->ubChannelNb,pPtr->eventData.LSB,pPtr->eventData.MSB);
	
	pitch_bend_2 (pPtr->ubChannelNb,pPtr->eventData.LSB,pPtr->eventData.MSB);
}

static inline void  fSetTempo(void *pEvent){
sTempo_EventBlock_t *pPtr=(sTempo_EventBlock_t *)pEvent;	
	amTrace((const U8*)"Setting new replay tempo...\n");
	
}


/* event id is mapped to the position in the array, functionPtr, parameters struct */

/** !ORDER IS IMPORTANT! and has to be the same as in enums with T_EVT_COUNT. Additionally
the ordering of members should be the same as described in type sEventList. */

static const sEventInfoBlock_t g_arSeqCmdTable[T_EVT_COUNT] = {
  {sizeof(sNoteOn_EventBlock_t),fNoteOn},
   {sizeof(sNoteOff_EventBlock_t),fNoteOff},
   {sizeof(sNoteAft_EventBlock_t), fNoteAft},
   {sizeof(sController_EventBlock_t),fController},
   {sizeof(sPrgChng_EventBlock_t),fProgramChange},
   {sizeof(sChannelAft_EventBlock_t),fChannelAft},
   {sizeof(sPitchBend_EventBlock_t),fPitchBend},
   {sizeof(sTempo_EventBlock_t),fSetTempo}
};

static inline void getEventFuncInfo(U8 eventType, sEventInfoBlock_t *infoBlk){
	
	infoBlk->size=g_arSeqCmdTable[eventType].size;
	infoBlk->func=g_arSeqCmdTable[eventType].func;
	
}

#endif

