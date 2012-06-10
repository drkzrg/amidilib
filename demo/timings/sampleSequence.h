
/** Copyright 2007-2012 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#ifndef __SAMPLE_SEQ_H__
#define __SAMPLE_SEQ_H__

typedef struct{
  U32 delta;
  U32 tempo;	// 0 == stop
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
  sTrack *tracks[3];	      //one per ym channel
  ePlayState state;	      //state 
  ePlayMode defaultPlayMode;  //play in loop, play once etc..
} sCurrentSequenceState;

//sample sequence data
// output, test sequence for channel 1 
  sEvent testSequenceChannel1[]={
  {0L,500,56,0xAD},
  {32L,500,127,0xAD},
  {32L,500,110,0xAD},
  {32L,500,127,0xAD},
  {32L,500,110,0xAD},
  {32L,500,127,0xAD},
  {32L,500,110,0xAD},
  {32L,500,127,0xAD},
  {32L,500,110,0xAD},
  {32L,500,127,0xAD},
  {32L,500,110,0xAD},
  {0,0,0,0xAD}
};

// output test sequence for channel 2
  sEvent testSequenceChannel2[]={
  {0L,500L,36,0xAD},
  {64L,500L,37,0xAD},
  {128L,500L,36,0xAD},
  {64L,500L,37,0xAD},
  {128L,500L,36,0xAD},
  {64L,500L,37,0xAD},
  {128L,500L,36,0xAD},
  {64L,500L,37,0xAD},
  {0L,500L,40,0xAD},
  {0L,500L,41,0xAD},
  {0L,500L,42,0xAD},
  {0L,500L,43,0xAD},
  {1L,500L,65,0xAD},
  {2L,500L,66,0xAD},
  {3L,500L,65,0xAD},
  {4L,500L,66,0xAD},
  {5L,500L,65,0xAD},
  {6L,500L,66,0xAD},
  {7L,500L,65,0xAD},
  {8L,500L,66,0xAD},
  {9L,500L,65,0xAD},
  {10L,500L,66,0xAD},
  {15L,500L,65,0xAD},
  {20L,500L,66,0xAD},
  {40L,500L,65,0xAD},
  {80L,500L,66,0xAD},
  {160L,500L,65,0xAD},
  {320L,500L,66,0xAD},
  {0L,0L,0,0xAD}
};

// output test sequence for channel 2
 sEvent testSequenceChannel3[]={
  {10L,500L,65,0xAD},
  {10L,500L,66,0xAD},
  {10L,500L,65,0xAD},
  {10L,500L,66,0xAD},
  {10L,500L,65,0xAD},
  {10L,500L,66,0xAD},
  {10L,500L,65,0xAD},
  {10L,500L,66,0xAD},
  {10L,500L,65,0xAD},
  {10L,500L,66,0xAD},
  {10L,500L,65,0xAD},
  {10L,500L,66,0xAD},
  {10L,500L,65,0xAD},
  {10L,500L,66,0xAD},
  {10L,500L,65,0xAD},
  {10L,500L,66,0xAD},
  {10L,500L,65,0xAD},
  {10L,500L,66,0xAD},
  {10L,500L,65,0xAD},
  {10L,500L,66,0xAD},
  {10L,500L,65,0xAD},
  {10L,500L,66,0xAD},
  {10L,500L,65,0xAD},
  {10L,500L,66,0xAD},
  {0L,0L,0,0xAD}
};

#endif
