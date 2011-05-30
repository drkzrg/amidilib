#ifndef __AMIDI_SEQ_H__
#define __AMIDI_SEQ_H__

/**  Copyright 2007-2011 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

/** file contains internal data structures for midi event internal storage and replay */

#include "midiseq.h"

#define AMIDI_MAX_TRACKS 16
#define EOT_SILENCE_THRESHOLD 40	/* after EOT_SILENCE_THRESHOLD delta increments and null events on all tracks */
					/* sequence is considered finished and ready to STOP or LOOP */

#define DEFAULT_TEMPO 0x007A120UL	/* 500000ms */
#define DEFAULT_PPQ	120		/* pulses per quarternote */
#define DEFAULT_TIME_SIG 0x0404	
	
/** sequence replay mode */
typedef enum{
  S_PLAY_ONCE=0x06,
  S_PLAY_LOOP=0x08,
  S_PLAY_RANDOM=0x10
} ePlayMode;

/** current track state */
typedef enum{
  PS_STOPPED=0x00,
  PS_PLAYING=0x02,
  PS_PAUSED=0x04
} ePlayState;

typedef struct EventList{
 struct EventList *pPrev,*pNext;
 sEventBlock_t eventBlock;
}PACK sEventList;

typedef volatile struct SequenceState_t{
 U32 currentTempo;		      // quaternote duration in ms, 500ms default
 U32 newTempo;			      // track new tempo in ms, if newTempo!=currentTempo
				      // then we have tempo change, which id handled in player
				      // and then currentTempo=newTempo;
				      
 U16 playState;			      // STOP, PLAY, PAUSED	
 U16 playMode;	      		      // current play mode (loop, play_once, random) 
				      // sets the active track, by default 0 
 U32 deltaCounter;		      // internal counter for sending events in proper time, relative to last event 				      
 struct EventList *pStart,*pCurrent;  //start of track and current event pointer
				      //they point to track events tied to sSequenceState_t
  BOOL bMute;			      //if TRUE track events aren't sent to external module
}PACK sSequenceState_t;

 typedef struct Track_t{
  U8 *pTrackName;			/* NULL terminated string with instrument name, track data and other text from MIDI meta events .. */
  sSequenceState_t currentState;        /* current sequence state */
  sEventList *pTrkEventList;  		/* track event list */
}PACK sTrack_t;

typedef struct Sequence_t{
   /** internal midi data storage format */
   U8 *pSequenceName;				/* NULL terminated string */
   sTrack_t *arTracks[AMIDI_MAX_TRACKS];	/* up to AMIDI_MAX_TRACKS (16) tracks available */
   U32 timeDivision;				// pulses per quater note(time division)
   U32 eotThreshold;				/*see define EOT_SILENCE_THRESHOLD */
   U8 ubNumTracks;            	        	/* number of tracks */
   U8 ubDummy[3];
   U8 ubActiveTrack; /* range 0-(ubNumTracks-1) tracks */
   U8 ubDummy1[3];
   } PACK sSequence_t;

#endif
