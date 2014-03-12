
/** Copyright 2007-2012 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#ifndef __SAMPLE_SEQ_H__
#define __SAMPLE_SEQ_H__

#include "c_vars.h"

typedef struct{
  U32 delta;
  U8 note;	// 0-127 range
  U8 dummy;
} sEvent; 

typedef struct{
  BOOL bIsActive;
  U8 volume;
  U8 dummy;
}sTrackState; 

typedef struct{
  sTrackState state;
  U32 timeElapsedInt;
  sEvent *seqPtr;
  U32 seqPosIdx;	//song position
} sTrack;

typedef struct{
  U32 currentTempo;	//quaternote duration in ms, 500ms default
  U32 currentPPQN;	//pulses per quater note
  U32 currentBPM;	//beats per minute (60 000000 / currentTempo)
  U32 timeElapsedFrac; //sequence elapsed time
  U32 timeStep; 	//sequence elapsed time
  sTrack tracks[3];	//one per ym channel
  U16 state;	//current state playing/paused/stopped
  U16 playMode;  //play in loop, play once etc..
}sCurrentSequenceState;

sEvent *getTestSequenceChannel(U8 chNb);

#endif
