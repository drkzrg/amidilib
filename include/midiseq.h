
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
	T_META_EOT,
	T_META_CUEPOINT,
	T_META_MARKER,
	T_SYSEX,
	T_EVT_COUNT		/* total number of events */
};

/******************** event block structs */
 
/*  Note On event block */
typedef struct NoteOn_EventBlock_t{
U8 ubChannelNb;			/* channel number */
sNoteOn_t eventData;	/*note on data */
} PACK sNoteOn_EventBlock_t;

/* Note Off event block */
typedef struct NoteOff_EventBlock_t{
U8 ubChannelNb;				/* channel number */
sNoteOff_t eventData;		/* note off data */
} PACK sNoteOff_EventBlock_t;

/* Note aftertouch eventblock */
typedef struct NoteAft_EventBlock_t{
 
 U8 ubChannelNb;
 sNoteAft_t eventData;
}PACK sNoteAft_EventBlock_t;

/* Controller change event block */
typedef struct Controller_EventBlock_t{
 U8 ubChannelNb;		/* channel number */
 sController_t eventData;	 /* controller event data */
} PACK sController_EventBlock_t;

/* Program change event block */
typedef struct PrgChng_EventBlock_t{
 U8 ubChannelNb;				/* channel number */
 sProgramChange_t eventData;	/* program change data */
} PACK sPrgChng_EventBlock_t;

/* Channel aftertouch eventblock */
typedef struct ChannelAft_EventBlock_t{  
  U8 ubChannelNb;				/* channel number */
  sChannelAft_t eventData;		/* channel after touch data */
}PACK sChannelAft_EventBlock_t;

/* Pitch bend eventblock */
typedef struct PitchBend_EventBlock_t{
 U8 ubChannelNb;			/* channel number  */
 sPitchBend_t eventData;    /* pitch bend data */
} PACK sPitchBend_EventBlock_t;

/* Set tempo eventblock */
typedef struct Tempo_EventBlock_t{
 sTempo_t eventData;	/* tempo event data  */
} PACK sTempo_EventBlock_t;

typedef struct Eot_EventBlock_t{
 U32 dummy;
} PACK sEot_EventBlock_t;

typedef struct CuePoint_EventBlock_t{
 U8 *pCuePointName;	/* cuepoint name, maybe something else */
} PACK sCuePoint_EventBlock_t;

typedef struct Marker_EventBlock_t{
 U8 *pMarkerName;	/* marker name, maybe something else */
} PACK sMarker_EventBlock_t;

typedef struct SysEX_EventBlock_t{
 U32 bufferSize ; 					/* size of SysEX buffer */
 U8 *pBuffer;						/* pointer to data */
} PACK sSysEX_EventBlock_t;


/** custom type evntFuncPtr for events in given sequence  */
typedef void (*evntFuncPtr)(void *pEvent);

/** IMPORTANT! structure EventInfoBlock_t is associated with table g_arSeqCmdTable[T_EVT_COUNT] 
    from MIDISEQ.C file changing the order of members here will affect structure of g_arSeqCmdTable[T_EVT_COUNT]. 
*/

typedef struct EventInfoBlock_t{
	U32	size;		/* size of command string in bytes */
	evntFuncPtr func;  	/* pointer to event handler */ 
} PACK sEventInfoBlock_t;

typedef struct EventBlock_t{
 U32 uiDeltaTime;					/* event delta time */	
 sEventInfoBlock_t sendEventCb;				/* send event function callback info block */ 
 sEventInfoBlock_t copyEventCb;			/* copy to internal buffer function callback info block */ 
 void *dataPtr;						/* pointer to event data of sEventInfoBlock_t.size * 1 byte (U8) */
 U8	type;						/* event type */
 }PACK sEventBlock_t, *sEventBlockPtr_t;

/** SysEX */
typedef struct SysEx_t{
 U32 bufferSize ; 					/* size of SysEX buffer */
 U8 *pBuffer;						/* pointer to data */
} PACK sSysEx_t;



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
	"Set tempo(Meta)",
	"End of track(Meta)",
	"Cuepoint(Meta)",
	"Marker(Meta)",
	"SysEX Msg"
};

//inline functions for sending data to external module
  const U8 *getEventName(U32 id);

  void fNoteOn (void *pEvent);
  void fNoteOff (void *pEvent);
  void fNoteAft (void *pEvent);
  void fProgramChange (void *pEvent);
  void fController (void *pEvent);
  void fChannelAft (void *pEvent);
  void fPitchBend (void *pEvent);
  void fSetTempo (void *pEvent);
  void fHandleEOT (void *pEvent);
  void fHandleCuePoint (void *pEvent);
  void fHandleMarker (void *pEvent);
  void fHandleSysEX (void *pEvent);
 
  void fNoteOnCopyData (void *pEvent);
  void fNoteOffCopyData (void *pEvent);
  void fNoteAftCopyData (void *pEvent);
  void fProgramChangeCopyData (void *pEvent);
  void fControllerCopyData (void *pEvent);
  void fChannelAftCopyData (void *pEvent);
  void fPitchBendCopyData (void *pEvent);
  void fHandleSysEXCopyData (void *pEvent);
  
/* returns the info struct about event: size and pointer to the handler  */
 void getEventFuncInfo (U8 eventType, sEventInfoBlock_t *infoBlk);

 /* returns the info struct about event: size and pointer to the handler  */
 void getEventFuncCopyInfo(U8 eventType, sEventInfoBlock_t *infoBlk);
 
/* returns channel number from info block (max 16 channels) or 127 if no channel info is available */
 U8 getChannelNbFromEventBlock (sEventBlock_t *pBlock);

#endif

