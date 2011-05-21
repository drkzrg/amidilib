#ifndef __AMIDI_SEQ_H__
#define __AMIDI_SEQ_H__

/**  Copyright 2007-2010 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

/** file contains internal data structures for midi event internal storage and replay */

#include "midiseq.h"

#define AMIDI_MAX_TRACKS 16

#define DEFAULT_TEMPO 0x007A120		/* 500000ms */
#define DEFAULT_PPQ	120		/* pulses per quarternote */
#define DEFAULT_TIME_SIG 0x0404	
	
/** current play mode */
typedef enum{
  S_PLAY_ONCE=0x00,
  S_PLAY_LOOP=0x02,
  S_PLAY_RANDOM=0x04
} ePlayMode;

/** current play mode */
typedef enum{
  S_STOPPED=0x00,
  S_PLAYING=0x02,
  S_PAUSED=0x04
} ePlayState;

typedef struct EventList{
 struct EventList *pPrev,*pNext;
 sEventBlock_t eventBlock;
}__attribute__((packed)) sEventList;

typedef struct SequenceState_t{
 volatile U32 currentTempo;			// quaternote duration in ms, 500ms default
 volatile U32 playState;			// STOP, PLAY, PAUSED	
 volatile U32 playMode;	      			// current play mode (loop, play_once, random) 
						// sets the active track, by default 0 
 volatile struct EventItem *pStart,*pCurrent;	//start of track and current event pointer
}__attribute__((packed)) sSequenceState_t;

 typedef struct Track_t{
  U8 *pInstrumentName;			/* NULL terminated string with instrument name, track data and other text from MIDI meta events .. */
  sSequenceState_t currentState;        /* current sequence state */
  sEventList trkEventList;  		/* track event list */
}__attribute__((packed)) sTrack_t;

typedef struct Sequence_t{
   /** internal midi data storage format */
   U8 *pSequenceName;				/* NULL terminated string */
   sTrack_t *arTracks[AMIDI_MAX_TRACKS];	/* up to AMIDI_MAX_TRACKS (16) tracks available */
   U32 timeDivision;				// pulses per quater note(time division)
   U8 ubNumTracks;            	        	/* number of tracks */
   U8 ubDummy[3];
   U8 ubActiveTrack;
   U8 ubDummy1[3];
 } __attribute__((packed)) sSequence_t;


#endif
