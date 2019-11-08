#ifndef __AMIDI_SEQ_H__
#define __AMIDI_SEQ_H__

/**  Copyright 2007-2019 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

/** file contains internal data structures for midi event internal storage and replay */

#include "midiseq.h"

#ifdef EVENT_LINEAR_BUFFER
#include "memory/linalloc.h"
#endif

#define AMIDI_MAX_TRACKS 16         // 16 should be enough

/** sequence type*/
typedef enum{
    ST_SINGLE=0,
    ST_MULTI,
    ST_MULTI_SUB
} eSequenceType;

// track state and play mode
typedef enum {
    // play mode
    TM_PLAY_ONCE  = 0b00000001, // play once if set, loop otherwise
    TM_MUTE = 0b10000000,       // mute track if set
    // track state
    TS_PS_PLAYING = 0b00000010, // playing if set, stopped otherwise
    TS_PS_PAUSED  = 0b00000100, // paused if set
} eTrackState;


typedef struct EventList{
  struct EventList *pPrev,*pNext;
  sEventBlock_t eventBlock;
} sEventList;

typedef struct TrackState_t{
 U32 currentTempo;		      // quaternote duration in ms, 500ms default
 U32 currentBPM;	          // beats per minute (60 000000 / currentTempo)
 U32 timeElapsedInt;		  // track elapsed time
 sEventList *currEventPtr;
 U16 playState;                   // bitfield with sequence state
                                  // sets the active track, by default 0
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
   U16 timeDivision;                        // pulses per quater note /PPQN /pulses per quaternote
   U16 ubNumTracks;                         // number of tracks 1->AMIDI_MAX_TRACKS */
   U16 ubActiveTrack;                       // range 0-(ubNumTracks-1) tracks */
   sTrack_t *arTracks[AMIDI_MAX_TRACKS];	// up to AMIDI_MAX_TRACKS tracks available */
   eSequenceType seqType;                   // sequence: single, multitrack, separate

#ifdef EVENT_LINEAR_BUFFER
   tLinearBuffer eventBuffer;               // contigous, linear buffer for midi events
#endif

} sSequence_t;

#endif

