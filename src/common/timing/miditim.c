
#include <mint/ostruct.h>
#include <mint/osbind.h>

#include "timing/miditim.h"
#include "amlog.h"

unsigned long begin;
unsigned long end;
long usp;


/* function for calculating tempo */
/* TODO: rewrite FPU version in asm in 060 and maybe 030 version */

/* called each time tempo is changed returned value is assigned to TimeStep value in sequence */
/* BPM - beats per minute (tempo of music) */
/* UPS - update interval (updates per second) */
/* PPQ - pulses per quaternote - music resolution */
/* music resolution are in PPQ */

U32 am_calculateTimeStep(U16 qpm, U16 ppq, U16 ups){
    U32 ppu;
    U32 temp;
    temp=(U32)qpm*(U32)ppq;
    
    if(temp<0x10000){
        ppu=((temp*0x10000)/60)/(U32)ups;
    }else{
        ppu=((temp/60)*0x10000)/(U32)ups;
    }
    
 return ppu;
}

/* function for calculating tempo (float version) */
/* called each time tempo is changed returned value is assigned to TimeStep value in sequence */
/* BPM - beats per minute (tempo of music) */
/* UPS - update interval (updates per second) */
/* music resolution are in PPQ */

float  am_calculateTimeStepFlt(U16 qpm, U16 ppq, U16 ups){
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
    amTrace((const U8*)"Timing (SMPTE): %x, %d\n", subframe,(timeDivision&0x00FF));
    return 0;		//todo:
  }
   else{
    /* PPQN */
    amTrace((const U8*)"Timing (PPQN): %d (0x%x)\n", timeDivision,timeDivision);
    return timeDivision;
   }
}


float getTimeStamp(){

// get Atari native system 200hz counter
	 usp=Super(0);
	 begin=*((long *)0x4ba);
	 SuperToUser(usp);
	
 return (float)begin;
}

float getTimeDelta(){
float delta=0.0f;

#ifdef TIME_CHECK_PORTABLE	 
     end=clock();
     delta=(float)(am_diffclock(end,begin)/1000.0f);
     return(delta);
#else
//calculate delta in seconds
    usp=Super(0);
    end=*((long *)0x4ba);
    SuperToUser(usp);
    delta=(end-begin)/200.0f;
    return(delta);
#endif
}

