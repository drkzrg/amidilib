
#include <mint/ostruct.h>
#include <mint/osbind.h>

#include "timing/miditim.h"
#include "amlog.h"

U32 begin;
U32 end;
long usp;

/* function for calculating tempo */
/* TODO: rewrite FPU version in asm in 060 and maybe 030 version */

/* function for calculating tempo (float version) */
/* called each time tempo is changed returned value is assigned to TimeStep value in sequence */
/* BPM - beats per minute (tempo of music) */
/* UPS - update interval (updates per second) */
/* music resolution are in PPQ */

float  am_calculateTimeStepFlt(const U16 qpm,const U16 ppq,const U16 ups){
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

U16 am_decodeTimeDivisionInfo(U16 timeDivision){
  U8 subframe=0;
  
  if(timeDivision&0x8000){
    
    /* SMPTE */
    timeDivision&=0x7FFF;
    subframe=timeDivision>>7;
    amTrace((const U8*)"Timing (SMPTE): %x, %u\n", subframe,(timeDivision&0x00FF));
    return 0;		//todo:
  }else{
    /* PPQN */
    amTrace((const U8*)"Timing (PPQN): %u\n", timeDivision);
    return timeDivision;
   }
}


U32 getTimeStamp(){

// get Atari native system 200hz counter
	 usp=Super(0);
	 begin=*((long *)0x4ba);
	 SuperToUser(usp);
	
 return begin;
}

U32 getTimeDelta(){
U32 delta=0.0f;

//calculate delta in seconds
    usp=Super(0);
    end=*((long *)0x4ba);
    SuperToUser(usp);
    delta=(end-begin)/200.0f;
    return(delta);
}

