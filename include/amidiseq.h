#ifndef __AMIDI_SEQ_H__
#define __AMIDI_SEQ_H__

/**  Copyright 2007-2011 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

/** file contains internal data structures for midi event internal storage and replay */

#include "midiseq.h"

#ifdef EVENT_LINEAR_BUFFER
#include "memory/linalloc.h"
#endif

#define AMIDI_MAX_TRACKS 64         // 64 should be enough
#define EOT_SILENCE_THRESHOLD 80	/* after EOT_SILENCE_THRESHOLD delta increments and null events on all tracks */
                                    /* sequence is considered finished and ready to STOP or LOOP */
/** sequence replay mode */
typedef enum{
  S_PLAY_ONCE=0x06,
  S_PLAY_LOOP=0x08,
  S_PLAY_RANDOM=0x10
} ePlayMode;

/** sequence type*/
typedef enum{
    ST_SINGLE=0,
    ST_MULTI,
    ST_MULTI_SUB
} eSequenceType;

/** current track state */
typedef enum{
  PS_STOPPED=0x00,
  PS_PLAYING=0x02,   
  PS_PAUSED=0x04 
} ePlayState;

typedef struct EventList{
 struct EventList *pPrev,*pNext;
 sEventBlock_t eventBlock;
} sEventList;

typedef struct TrackState_t{
 U32 currentTempo;		      // quaternote duration in ms, 500ms default
 U32 currentBPM;	          // beats per minute (60 000000 / currentTempo)
 U32 timeElapsedInt;		  // track elapsed time

 sEventList *currEventPtr;
 ePlayState playState;		  // STOP, PLAY, PAUSED
 ePlayMode playMode;	      // current play mode (loop, play_once, random)
                              // sets the active track, by default 0
 BOOL bMute;			      // if TRUE track events aren't sent to external module
} sTrackState_t;

 typedef struct Track_t{
  sTrackState_t currentState;       /* current sequence state */
  sEventList *pTrkEventList;  		/* track event list */
  U8 *pTrackName;                   /* NULL terminated string with instrument name, track data and other text from MIDI meta events .. */
} sTrack_t;

typedef struct Sequence_t{
   /** internal midi data storage format */
   U8 *pSequenceName;                       // NULL terminated string */
   U32 timeElapsedFrac;                     // sequence elapsed time
   U32 timeStep;                            // sequence time step
   U32 eotThreshold;                        // see define EOT_SILENCE_THRESHOLD */
   U16 timeDivision;                        // pulses per quater note /PPQN /pulses per quaternote
   U16 ubNumTracks;                         // number of tracks 1->AMIDI_MAX_TRACKS */
   U16 ubActiveTrack;                       // range 0-(ubNumTracks-1) tracks */
   sTrack_t *arTracks[AMIDI_MAX_TRACKS];	// up to AMIDI_MAX_TRACKS tracks available */
   eSequenceType seqType;                   // sequence: single, multitrack, separate

#ifdef EVENT_LINEAR_BUFFER
   tLinearBuffer eventBuffer;               // contigous, linear buffer for midi events
#endif

} sSequence_t;

// timer type on which update will be executed
typedef enum{
  MFP_TiA=0,
  MFP_TiB,
  MFP_TiC
} eTimerType;

#endif

