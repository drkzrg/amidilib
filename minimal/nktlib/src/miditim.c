
#include <mint/ostruct.h>
#include <mint/osbind.h>

#include "timing/miditim.h"
#include "amlog.h"

/* functions for calculating tempo */

/* support functions:
    BPM = 60,000,000/MicroTempo
    MicrosPerPPQN = MicroTempo/TimeBase
    MicrosPerMIDIClock = MicroTempo/24
    PPQNPerMIDIClock = TimeBase/24
    MicrosPerSubFrame = 1000000 * Frames * SubFrames
    SubFramesPerQuarterNote = MicroTempo/(Frames * SubFrames)
    SubFramesPerPPQN = SubFramesPerQuarterNote/TimeBase
    MicrosPerPPQN = SubFramesPerPPQN * Frames * SubFrames
*/

uint16 amDecodeTimeDivisionInfo(uint16 timeDivision){
  
  if(timeDivision&0x8000){
    
    /* SMPTE */
	uint8 subframe=0;
    timeDivision&=0x7FFF;
    subframe=timeDivision>>7;
    amTrace((const uint8*)"Timing (SMPTE): %x, %u\n", subframe,(timeDivision&0x00FF));
    return 0;		//todo:
  }else{
    /* PPQN */
    amTrace((const uint8*)"Timing (PPQN): %u\n", timeDivision);
    return timeDivision;
   }
}

static uint32 begin;
static  uint32 end;
static  int32 usp;

void amGetTimeStamp(void){
// get Atari native system 200hz counter
     usp=Super(0L);
     begin=*((int32 *)0x4ba);
     SuperToUser(usp);
}

uint32 amGetTimeDelta(void){
uint32 delta=0L;

//calculate delta in seconds
    usp=Super(0L);
    end=*((int32 *)0x4ba);
    delta=(end-begin)/200;
    SuperToUser(usp);

    return(delta);
}

