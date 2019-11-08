
#ifndef __MIDISEQ_H__
#define __MIDISEQ_H__

/**  Copyright 2007-2019 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#include "c_vars.h"
#include "midi.h"
#include "midi_cmd.h"	/* for sending midi commands */

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
	T_META_EOT,
	T_META_CUEPOINT,
	T_META_MARKER,
	T_META_SET_SIGNATURE,
	T_SYSEX,
	T_EVT_COUNT		/* total number of events */
};

/******************** event block structs */
 
/*  Note On event block */
typedef struct NoteOn_EventBlock_t{
uint8 ubChannelNb;			/* channel number */
sNoteOn_t eventData;	/*note on data */
} sNoteOn_EventBlock_t;

/* Note Off event block */
typedef struct NoteOff_EventBlock_t{
uint8 ubChannelNb;				/* channel number */
sNoteOff_t eventData;		/* note off data */
} sNoteOff_EventBlock_t;

/* Note aftertouch eventblock */
typedef struct NoteAft_EventBlock_t{
 
 uint8 ubChannelNb;
 sNoteAft_t eventData;
} sNoteAft_EventBlock_t;

/* Controller change event block */
typedef struct Controller_EventBlock_t{
 uint8 ubChannelNb;		/* channel number */
 sController_t eventData;	 /* controller event data */
} sController_EventBlock_t;

/* Program change event block */
typedef struct PrgChng_EventBlock_t{
 uint8 ubChannelNb;				/* channel number */
 sProgramChange_t eventData;	/* program change data */
} sPrgChng_EventBlock_t;

/* Channel aftertouch eventblock */
typedef struct ChannelAft_EventBlock_t{  
  uint8 ubChannelNb;				/* channel number */
  sChannelAft_t eventData;		/* channel after touch data */
} sChannelAft_EventBlock_t;

/* Pitch bend eventblock */
typedef struct PitchBend_EventBlock_t{
 uint8 ubChannelNb;			/* channel number  */
 sPitchBend_t eventData;    /* pitch bend data */
} sPitchBend_EventBlock_t;

/* Set tempo eventblock */
typedef struct Tempo_EventBlock_t{
 sTempo_t eventData;	/* tempo event data  */
} sTempo_EventBlock_t;

/* */
typedef struct Eot_EventBlock_t{
 uint32 dummy;
} sEot_EventBlock_t;

typedef struct CuePoint_EventBlock_t{
 uint8 *pCuePointName;	/* cuepoint name, maybe something else */
} sCuePoint_EventBlock_t;

typedef struct Marker_EventBlock_t{
 uint8 *pMarkerName;	/* marker name, maybe something else */
} sMarker_EventBlock_t;

typedef struct TimeSignature_EventBlock_t{
 sTimeSignature timeSignature;
} sTimeSignature_EventBlock_t;

typedef struct SysEX_EventBlock_t{
 uint32 bufferSize ; 					/* size of SysEX buffer */
 uint8 *pBuffer;						/* pointer to data */
} sSysEX_EventBlock_t;


/** custom type evntFuncPtr for events in given sequence  */
typedef void (*evntFuncPtr)(const void *pEvent);

/** IMPORTANT! structure EventInfoBlock_t is associated with table g_arSeqCmdTable[T_EVT_COUNT] 
    from MIDISEQ.C file changing the order of members here will affect structure of g_arSeqCmdTable[T_EVT_COUNT]. 
*/

typedef struct EventInfoBlock_t{
	uint32	size;		/* size of command string in bytes */
	evntFuncPtr func;  	/* pointer to event handler */ 
} sEventInfoBlock_t;

typedef struct EventBlock_t{
 uint32 uiDeltaTime;				/* event delta time */
 uint8 type;						/* event type */
 uint8 pad0;                       /* padding */

#ifdef IKBD_MIDI_SEND_DIRECT
 sEventInfoBlock_t copyEventCb;		/* copy to internal buffer function callback info block */
#else
 sEventInfoBlock_t sendEventCb;		/* send event function callback info block */
#endif

 void *dataPtr;						/* pointer to event data of sEventInfoBlock_t.size * 1 byte (uint8) */
} sEventBlock_t, *sEventBlockPtr_t;

/** SysEX */
typedef struct SysEx_t{
 uint32 bufferSize ; 					/* size of SysEX buffer */
 uint8 *pBuffer;						/* pointer to data */
} sSysEx_t;

//inline functions for sending data to external module
const uint8 *getEventName(uint32 id);

#ifdef IKBD_MIDI_SEND_DIRECT
 /* returns the info struct about event: size and pointer to the handler  */
 void getEventFuncCopyInfo(const uint8 eventType, sEventInfoBlock_t *infoBlk);
#else
 /* returns the info struct about event: size and pointer to the handler  */
  void getEventFuncInfo (const uint8 eventType, sEventInfoBlock_t *infoBlk);
#endif

/* returns channel number from info block (max 16 channels) or 127 if no channel info is available */
 uint8 getChannelNbFromEventBlock (const sEventBlock_t *pBlock);

#endif

