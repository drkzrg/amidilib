
#include <mint/ostruct.h>
#include <mint/osbind.h>

#include "timing/miditim.h"
#include "amlog.h"

/* function for calculating tempo */
/* TODO: rewrite FPU version in asm in 060 and maybe 030 version */

/* function for calculating tempo (float version) */
/* called each time tempo is changed returned value is assigned to TimeStep value in sequence */
/* BPM - beats per minute (tempo of music) */
/* UPS - update interval (updates per second) */
/* music resolution are in PPQ */

float  am_calculateTimeStepFlt(const uint16 qpm,const uint16 ppq,const uint16 ups){
    float ppu=0;
    float temp=0;
    ppu=(float)qpm*(float)ppq;
    temp=(temp/ups)/60.0f;
   
 return ppu;
}

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

uint16 am_decodeTimeDivisionInfo(uint16 timeDivision){
  
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

void getTimeStamp(void){
// get Atari native system 200hz counter
     usp=Super(0L);
     begin=*((int32 *)0x4ba);
     SuperToUser(usp);
}

uint32 getTimeDelta(void){
uint32 delta=0L;

//calculate delta in seconds
    usp=Super(0L);
    end=*((int32 *)0x4ba);
    delta=(end-begin)/200;
    SuperToUser(usp);

    return(delta);
}

