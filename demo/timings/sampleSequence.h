
/** Copyright 2007-2012 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#ifndef __SAMPLE_SEQ_H__
#define __SAMPLE_SEQ_H__

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
  ePlayState state;	//current state playing/paused/stopped 
  ePlayMode playMode;  //play in loop, play once etc..
  
} sCurrentSequenceState;

//sample sequence data
// output, test sequence for channel 1 
  static sEvent testSequenceChannel1[]={
  {0L,128,0xAD},
  {16L,110,0xAD},
  {32L,128,0xAD},
  {64L,46,0xAD},
  {96L,128,0xAD},
  {128L,46,0xAD},
  {0L,128,0xAD},
  {16L,46,0xAD},
  {32L,128,0xAD},
  {64L,46,0xAD},
  {96L,128,0xAD},
  {128L,46,0xAD},
  {0L,128,0xAD},
  {16L,46,0xAD},
  {32L,128,0xAD},
  {64L,46,0xAD},
  {96L,128,0xAD},
  {128L,46,0xAD},
  {0L,128,0xAD},
  {16L,46,0xAD},
  {32L,128,0xAD},
  {64L,46,0xAD},
  {96L,128,0xAD},
  {128L,46,0xAD},
  {0,0,0xAD}
};

// output test sequence for channel 2
 static sEvent testSequenceChannel2[]={
  {0L,128,0xAD},
  {16L,110,0xAD},
  {32L,128,0xAD},
  {64L,46,0xAD},
  {96L,128,0xAD},
  {128L,46,0xAD},
  {0L,128,0xAD},
  {16L,46,0xAD},
  {32L,128,0xAD},
  {64L,46,0xAD},
  {96L,128,0xAD},
  {128L,46,0xAD},
  {0L,128,0xAD},
  {16L,46,0xAD},
  {32L,128,0xAD},
  {64L,46,0xAD},
  {96L,128,0xAD},
  {128L,46,0xAD},
  {0L,128,0xAD},
  {16L,46,0xAD},
  {32L,128,0xAD},
  {64L,46,0xAD},
  {96L,128,0xAD},
  {128L,46,0xAD},
  {0,0,0xAD}
};

// output test sequence for channel 2
 static sEvent testSequenceChannel3[]={
  {0L,128,0xAD},
  {16L,110,0xAD},
  {32L,128,0xAD},
  {64L,46,0xAD},
  {96L,128,0xAD},
  {128L,46,0xAD},
  {0L,128,0xAD},
  {16L,46,0xAD},
  {32L,128,0xAD},
  {64L,46,0xAD},
  {96L,128,0xAD},
  {128L,46,0xAD},
  {0L,128,0xAD},
  {16L,46,0xAD},
  {32L,128,0xAD},
  {64L,46,0xAD},
  {96L,128,0xAD},
  {128L,46,0xAD},
  {0L,128,0xAD},
  {16L,46,0xAD},
  {32L,128,0xAD},
  {64L,46,0xAD},
  {96L,128,0xAD},
  {128L,46,0xAD},
  {0,0,0xAD}
};

#endif
