
/**  Copyright 2007-2010 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#include "ym2149.h"
#include "midi_cmd.h"
  
void ymDoSound(ymChannelData ch[3],U8 envelope, U16 envPeriod,U8 noiseGenPeriod){
  
    U8 mixerSet=0b11111111; 				// all off
    
    if(ch[CH_A].noiseEnable)mixerSet&=0b11110111;
    if(ch[CH_A].toneEnable) mixerSet&=0b11111110;
    if(ch[CH_B].noiseEnable)mixerSet&=0b11101111;
    if(ch[CH_B].toneEnable) mixerSet&=0b11111101;
    if(ch[CH_C].noiseEnable)mixerSet&=0b11011111;
    if(ch[CH_C].toneEnable) mixerSet&=0b11111011;
#ifndef PORTABLE      
    Giaccess(mixerSet,MIXER+128);		//sound chip setup
						//enable 3 oscillators on all channels
					
    Giaccess(ch[CH_A].amp,AMP_OSC1+128);	// set osc1 CH A
    Giaccess(ch[CH_A].oscFreq,LB_OSC1+128);	
    Giaccess(ch[CH_A].oscStepSize,HB_OSC1+128);
    
    Giaccess(ch[CH_B].amp,AMP_OSC2+128);	// set osc2 CH B
    Giaccess(ch[CH_B].oscFreq,LB_OSC2+128);	
    Giaccess(ch[CH_B].oscStepSize,HB_OSC2+128);
    
    Giaccess(ch[CH_C].amp,AMP_OSC3+128);	// set osc3	CH C
    Giaccess(ch[CH_C].oscFreq,LB_OSC3+128);	
    Giaccess(ch[CH_B].oscStepSize,HB_OSC3+128);
 
    //set noise generator period 0-31
    if(noiseGenPeriod>31) noiseGenPeriod=31;
    Giaccess(noiseGenPeriod,NOISE_GEN+128);
#endif    
    //set envelope period
    U8 lPeriod=(U8)(0x00FF&envPeriod);
    U8 hPeriod=(U8)(0x00FF&(envPeriod>>4));
#ifndef PORTABLE    
    Giaccess(lPeriod,LB_ENV_PERIOD+128);
    Giaccess(hPeriod,HB_ENV_PERIOD+128);
    
    //set envelopes
    Giaccess(envelope,ENV_SELECT+128);
#endif   
}

void ymSoundOff(){
#ifndef PORTABLE
  Giaccess(0b11111111,MIXER+128);
  
  //turn off envelope period
  Giaccess(0,LB_ENV_PERIOD+128);
  Giaccess(0,HB_ENV_PERIOD+128);

  Giaccess(0,AMP_OSC1+128);		// set osc1
  Giaccess(0,LB_OSC1+128);	
  Giaccess(0,HB_OSC1+128);
    
  Giaccess(0,AMP_OSC2+128);		// set osc2
  Giaccess(0,LB_OSC2+128);	
  Giaccess(0,HB_OSC2+128);
    
  Giaccess(0,AMP_OSC3+128);		// set osc3
  Giaccess(0,LB_OSC3+128);	
  Giaccess(0,HB_OSC3+128);
#endif
}

void setYm2149(ymChannelData ch[3],int noteIdx,U8 currentEnvelopeIdx, U8 noisegenPeriod){
     U8 hByte=g_arMIDI2ym2149Tone[noteIdx].highbyte;
     U8 lByte=g_arMIDI2ym2149Tone[noteIdx].lowbyte;
     U8 envelope=envelopeArray[currentEnvelopeIdx];
     U16 period=g_arMIDI2ym2149Tone[noteIdx].period;
	  
     ch[CH_A].oscFreq=lByte;
     ch[CH_A].oscStepSize=hByte;
     ch[CH_B].oscFreq=lByte;
     ch[CH_B].oscStepSize=hByte;
     ch[CH_C].oscFreq=lByte;
     ch[CH_C].oscStepSize=hByte;
	    
     ymDoSound(ch,envelope,period,noisegenPeriod);
}

//plays given note and outputs it to midi/ym2149
void playNote(U8 noteNb, BOOL bMidiOutput, BOOL bYmOutput){
ymChannelData ch[3];

  if(bMidiOutput==TRUE){
    note_on(9,noteNb,127);	//output on channel 9, max velocity
  }

  if(bYmOutput==TRUE){

     U8 hByte=g_arMIDI2ym2149Tone[noteNb].highbyte;
     U8 lByte=g_arMIDI2ym2149Tone[noteNb].lowbyte;
     U16 period=g_arMIDI2ym2149Tone[noteNb].period;
	  
     ch[CH_A].oscFreq=lByte;
     ch[CH_A].oscStepSize=hByte;
     ch[CH_B].oscFreq=lByte;
     ch[CH_B].oscStepSize=hByte;
     ch[CH_C].oscFreq=lByte;
     ch[CH_C].oscStepSize=hByte;
    
    ymDoSound( ch,4 , period,128);
    
  }

}
