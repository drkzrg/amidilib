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
 U32 uiNbOfItems;
 sEventItem *pEventList;
} sEventList;

typedef struct sTrackState{
 U32 currentPos;              /* current delta position, 0 at start */
 U8 ubVolume;                 /* sequence current track volume */
 U32 ulTimeStep;              /* sequence current update time step */
 U32 ulTrackTempo;	      /* miliseconds per quarter note, default 500 000ms */
} sTrackState;

typedef struct Track_t{
  U8 *pInstrumentName;		/* NULL terminated string with instrument name, track data and other text from MIDI meta events .. */
  sTrackState currTrackState;	/* current track state */
  sEventList trkEventList;  	/* track event list */
} sTrack_t;

typedef struct SequenceState_t{
 U8 ubVolume;                 /* sequence master volume */
 U8 ubActiveTrack;            /* sets the active track, by default 0 */
 U8 bPlayState;		      /* indicates if sequence is currently being played or not: 
				S_STOPPED (stopped), 
				S_PLAYING (playing),
				S_PAUSED (paused)sequence is being played / FALSE */          
 U8 ubPlayModeState;	      /* current play mode (loop, play_once) */
 U32 timeElapsed;	      /* TODO: move time/timeElapsed to global space (?)*/
 U32 time;
} sSequenceState_t;

typedef struct Sequence_t{
   /** internal midi data storage format */
   U8 *pSequenceName;			/* NULL terminated string */
   U8 ubNumTracks;            	        /* number of tracks */
   U16 uiTimeDivision;			/* PPQN */
   sTrack_t *arTracks[AMIDI_MAX_TRACKS];/* up to AMIDI_MAX_TRACKS tracks available */
   sSequenceState_t currentState;       /* current sequence state */
} sSequence_t;


/** Plays loaded sequence
 
*/
/** returns next delta */
U32 am_playSequence(sSequence_t *pTune,U32 start_delta, U32 playMode);

#endif
