
/** Copyright 2007-2020 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#ifndef TIM_TEST_SAMPLE_SEQ_H_
#define TIM_TEST_SAMPLE_SEQ_H_

#include "c_vars.h"

typedef struct{
  uint32 delta;
  uint8 note;	// 0-127 range
  uint8 dummy;
} sEvent; 

typedef struct{
  bool bIsActive;
  uint8 volume;
  uint8 dummy;
}sTrackState; 

typedef struct{
  sTrackState state;
  uint32 timeElapsedInt;
  sEvent *seqPtr;
  uint32 seqPosIdx;	//song position
} sTrack;

typedef struct{
  uint32 currentTempo;	//quaternote duration in ms, 500ms default
  uint32 currentPPQN;	//pulses per quater note
  uint32 currentBPM;	//beats per minute (60 000000 / currentTempo)
  uint32 timeElapsedFrac; //sequence elapsed time
  uint32 timeStep; 	//sequence elapsed time
  sTrack tracks[3];	//one per ym channel
  uint16 state;	//current state playing/paused/stopped
  uint16 playMode;  //play in loop, play once etc..
}sCurrentSequenceState;

sEvent *getTestSequenceChannel(const uint8 chNb);

typedef enum{
 S_PLAY_LOOP=0,
 S_PLAY_ONCE=1,
 PS_PAUSED=2,
 PS_PLAYING=3,
 PS_STOPPED=4
}ePlayModeStates;

#endif
