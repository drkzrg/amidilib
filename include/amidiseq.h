#ifndef __AMIDI_SEQ_H__
#define __AMIDI_SEQ_H__

/**  Copyright 2007-2011 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

/** file contains internal data structures for midi event internal storage and replay */

#include "midiseq.h"

#define AMIDI_MAX_TRACKS 65536
#define EOT_SILENCE_THRESHOLD 80	/* after EOT_SILENCE_THRESHOLD delta increments and null events on all tracks */
					/* sequence is considered finished and ready to STOP or LOOP */
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

typedef struct TrackState_t{
 sTimeSignature timeSignature;	      //time signature
 U32 currentTempo;		      //quaternote duration in ms, 500ms default
 U32 currentPPQN;		      //pulses per quater note
 U32 currentBPM;	              //beats per minute (60 000000 / currentTempo)
 U32 currentSeqPos;		      //index of current event in list
 U32 timeElapsedInt;		      //track elapsed time
 ePlayState playState;		      // STOP, PLAY, PAUSED	
 ePlayMode playMode;	      	      // current play mode (loop, play_once, random) 
				      // sets the active track, by default 0 
 BOOL bMute;			      // if TRUE track events aren't sent to external module
}PACK sTrackState_t;

 typedef struct Track_t{
  sTrackState_t currentState;        /* current sequence state */
  sEventList *pTrkEventList;  		/* track event list */
  U8 *pTrackName;			/* NULL terminated string with instrument name, track data and other text from MIDI meta events .. */
}PACK sTrack_t;

typedef struct Sequence_t{
   /** internal midi data storage format */
   U8 *pSequenceName;				/* NULL terminated string */
   U32 timeElapsedFrac; 			//sequence elapsed time
   U32 timeStep; 				//sequence time step
   U32 eotThreshold;				/* see define EOT_SILENCE_THRESHOLD */
   U16 ubNumTracks;            	        /* number of tracks 1-65536 */
   U16 ubActiveTrack; 				/* range 0-(ubNumTracks-1) tracks */
   sTrack_t *arTracks[AMIDI_MAX_TRACKS];	/* up to AMIDI_MAX_TRACKS (16) tracks available */
} PACK sSequence_t;

#endif
