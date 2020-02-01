
/**  Copyright 2007-2019 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#include "ym2149.h"
#include "midi_cmd.h"

#include <osbind.h>


/* MIDI notes to ym2149 tone mapping */
/* notes are mapped from 21 to 108 */
/* highbyte, low byte (in one word)*/

static const ymData g_arMIDI2ym2149Tone[128]={
/*0 */ {0x0000,0x00,0x00}, /*undefined */
/*1 */ {0x0000,0x00,0x00}, /*undefined */
/*2 */ {0x0000,0x00,0x00}, /*undefined */
/*3 */ {0x0000,0x00,0x00}, /*undefined */
/*4 */ {0x0000,0x00,0x00}, /*undefined */
/*5 */ {0x0000,0x00,0x00}, /*undefined */
/*6 */ {0x0000,0x00,0x00}, /*undefined */
/*7 */ {0x0000,0x00,0x00}, /*undefined */
/*8 */ {0x0000,0x00,0x00}, /*undefined */
/*9*/ {0x0000,0x00,0x00}, /*undefined */
/*10 */ {0x0000,0x00,0x00}, /*undefined */
/*11 */ {0x0000,0x00,0x00}, /*undefined */
/*12 */ {0x0000,0x00,0x00}, /*undefined */
/*13 */ {0x0000,0x00,0x00}, /*undefined */
/*14 */ {0x0000,0x00,0x00}, /*undefined */
/*15 */ {0x0000,0x00,0x00}, /*undefined */
/*16 */ {0x0000,0x00,0x00}, /*undefined */
/*17 */ {0x0000,0x00,0x00}, /*undefined */
/*18 */ {0x0000,0x00,0x00}, /*undefined */
/*19 */ {0x0000,0x00,0x00}, /*undefined */
/*20 */ {0x0000,0x00,0x00}, /*undefined */
/*21*/ {0x0000,0x00,0x00}, /*undefined */
/*22*/ {0x0000,0x00,0x00}, /*undefined */
/*23*/ {0x0000,0x00,0x00},  /*undefined */
/*24*/ {3822,14,238},
/*25*/ {3607,14,23},
/*26*/ {3405,13,77},
/*27*/ {3214,12,142},
/*28*/ {3033,11,217},
/*29*/ {2863,11,47},
/*30*/ {2702,10,142},
/*31*/ {2551,9,247},
/*32*/ {2407,9,103},
/*33*/ {2272,8,224},
/*34*/ {2145,8,97},
/*35*/ {2024,7,232},
/*36*/ {1911,7,119},
/*37*/ {1803,7,11},
/*38*/ {1702,6,166},
/*39*/ {1607,6,71},
/*40*/ {1516,5,236},
/*41*/ {1431,5,151},
/*42*/ {1351,5,71},
/*43*/ {1275,4,251},
/*44*/ {1203,4,179},
/*45*/ {1136,4,112},
/*46*/ {1072,4,48},
/*47*/ {1012,3,244},
/*48*/ {955,3,187},
/*49*/ {901,3,133},
/*50*/ {851,3,83},
/*51*/ {803,3,35},
/*52*/ {758,2,246},
/*53*/ {715,2,203},
/*54*/ {675,2,163},
/*55*/ {637,2,125},
/*56*/ {601,2,89},
/*57*/ {568,2,56},
/*58*/ {536,2,24},
/*59*/ {506,1,250},
/*60*/ {477,1,221},
/*61*/ {450,1,194},
/*62*/ {425,1,169},
/*63*/ {401,1,145},
/*64*/ {279,1,123},
/*65*/ {357,1,101},
/*66*/ {337,1,81},
/*67*/ {318,1,62},
/*68*/ {300,1,44},
/*69*/ {284,1,28},
/*70*/ {268,1,12},
/*71*/ {253,0,153},
/*72*/ {238,0,238},
/*73*/ {225,0,225},
/*74*/ {212,0,212},
/*75*/ {200,0,200},
/*76*/ {189,0,189},
/*77*/ {178,0,178},
/*78*/ {168,0,168},
/*79*/ {159,0,159},
/*80*/ {150,0,150},
/*81*/ {142,0,142},
/*82*/ {134,0,134},
/*83*/ {126,0,126},
/*84*/ {119,0,119},
/*85*/ {112,0,112},
/*86*/ {106,0,106},
/*87*/ {100,0,100},
/*88*/ {94,0,94},
/*89*/ {89,0,89},
/*90*/ {84,0,84},
/*91*/ {79,0,79},
/*92*/ {75,0,75},
/*93*/ {71,0,71},
/*94*/ {67,0,67},
/*95*/ {63,0,63},
/*96*/ {59,0,59},
/*97*/ {56,0,56},
/*98*/ {53,0,53},
/*99*/ {50,0,53},
/*100*/ {50,0,50},
/*101*/ {47,0,47},
/*102*/ {44,0,44},
/*103*/ {42,0,42},
/*104*/ {39,0,39},
/*105*/ {37,0,37},
/*106*/ {35,0,35},
/*107*/ {33,0,33},
/*108*/ {31,0,31},
/*109*/ {29,0,29},
/*110*/ {28,0,28},
/*111*/ {26,0,26},
/*112*/ {25,0,25},
/*113*/ {23,0,23},
/*114*/ {22,0,22},
/*115*/ {21,0,21},
/*116*/ {19,0,19},
/*117*/ {18,0,18},
/*118*/ {17,0,17},
/*119*/ {16,0,16},
/* 120 */ {15,0,15},
/* 121 */ {0x0000,0x00,0x00}, /* undefined */
/* 122 */ {0x0000,0x00,0x00}, /* undefined */
/* 123 */ {0x0000,0x00,0x00}, /* undefined */
/* 124 */ {0x0000,0x00,0x00}, /* undefined */
/* 125 */ {0x0000,0x00,0x00}, /* undefined */
/* 126 */ {0x0000,0x00,0x00}, /* undefined */
/* 127 */ {0x0000,0x00,0x00}  /* undefined */
};

static const uint8 envelopeArray[8]={ENV_1,ENV_2,ENV_3,ENV_4,ENV_5,ENV_6,ENV_7,ENV_8 };

eYMenvelope getEnvelopeId(const uint8 id){
  return envelopeArray[id];
}


void ymDoSound(ymChannelData ch[3],uint8 envelope, uint16 envPeriod,uint8 noiseGenPeriod,uint8 affectChannels){
  
    uint8 mixerSet=0b11111111; 				// all off
    mixerSet=Giaccess(0,MIXER);
    
    if(affectChannels==CH_A||affectChannels==CH_ALL){
      if(ch[CH_A].noiseEnable)mixerSet&=0b11110111;
      if(ch[CH_A].toneEnable) mixerSet&=0b11111110;
    }
    
     if(affectChannels==CH_B||affectChannels==CH_ALL){
      if(ch[CH_B].noiseEnable)mixerSet&=0b11101111;
      if(ch[CH_B].toneEnable) mixerSet&=0b11111101;
    }
    
    if(affectChannels==CH_C||affectChannels==CH_ALL){
      if(ch[CH_C].noiseEnable)mixerSet&=0b11011111;
      if(ch[CH_C].toneEnable) mixerSet&=0b11111011;
    }
    Giaccess(mixerSet,MIXER+128);		//sound chip setup
						//enable 3 oscillators on all channels
    
    if(affectChannels==CH_A||affectChannels==CH_ALL){
      Giaccess(ch[CH_A].amp,AMP_OSC1+128);	// set osc1 CH A
      Giaccess(ch[CH_A].oscFreq,LB_OSC1+128);	
      Giaccess(ch[CH_A].oscStepSize,HB_OSC1+128);
    }else{
      ch[CH_A].amp=Giaccess(0,AMP_OSC1);	// get osc1 CH A
      ch[CH_A].oscFreq=Giaccess(0,LB_OSC1);	
      ch[CH_A].oscStepSize=Giaccess(0,HB_OSC1);
    }
    
    if(affectChannels==CH_B||affectChannels==CH_ALL){
      Giaccess(ch[CH_B].amp,AMP_OSC2+128);	// set osc2 CH B
      Giaccess(ch[CH_B].oscFreq,LB_OSC2+128);	
      Giaccess(ch[CH_B].oscStepSize,HB_OSC2+128);
    }else{
      ch[CH_B].amp=Giaccess(0,AMP_OSC2);	// set osc2 CH B
      ch[CH_B].oscFreq=Giaccess(0,LB_OSC2);	
      ch[CH_B].oscStepSize=Giaccess(0,HB_OSC2);
    }
    
    if(affectChannels==CH_C||affectChannels==CH_ALL){
      Giaccess(ch[CH_C].amp,AMP_OSC3+128);	// set osc3	CH C
      Giaccess(ch[CH_C].oscFreq,LB_OSC3+128);	
      Giaccess(ch[CH_C].oscStepSize,HB_OSC3+128);
    }else{
      ch[CH_C].amp=Giaccess(0,AMP_OSC3);	// get osc3	CH C
      ch[CH_C].oscFreq=Giaccess(0,LB_OSC3);	
      ch[CH_C].oscStepSize=Giaccess(0,HB_OSC3);
    }
    
    //set noise generator period 0-31
    if(noiseGenPeriod>31) noiseGenPeriod=31;
    Giaccess(noiseGenPeriod,NOISE_GEN+128);

    //set envelope period
    uint8 lPeriod=(uint8)(0x00FF&envPeriod);
    uint8 hPeriod=(uint8)(0x00FF&(envPeriod>>4));

    Giaccess(lPeriod,LB_ENV_PERIOD+128);
    Giaccess(hPeriod,HB_ENV_PERIOD+128);
    
    //set envelopes
    Giaccess(envelope,ENV_SELECT+128);
}

void ymSoundOff(void){
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
}

void setYm2149(ymChannelData ch[3],const int noteIdx,const uint8 currentEnvelopeIdx, const uint8 noisegenPeriod){
     uint8 hByte=g_arMIDI2ym2149Tone[noteIdx].highbyte;
     uint8 lByte=g_arMIDI2ym2149Tone[noteIdx].lowbyte;
     uint8 envelope=envelopeArray[currentEnvelopeIdx];
     uint16 period=g_arMIDI2ym2149Tone[noteIdx].period;
	  
     ch[CH_A].oscFreq=lByte;
     ch[CH_A].oscStepSize=hByte;
     ch[CH_B].oscFreq=lByte;
     ch[CH_B].oscStepSize=hByte;
     ch[CH_C].oscFreq=lByte;
     ch[CH_C].oscStepSize=hByte;
	    
     ymDoSound(ch,envelope,period,noisegenPeriod,(uint8)CH_ALL);
}

//plays given note and outputs it to midi/ym2149
void playNote(const uint8 channel,const uint8 noteNb, const bool bMidiOutput, const bool bYmOutput){
ymChannelData ch[3];

  
     uint8 hByte=g_arMIDI2ym2149Tone[noteNb].highbyte;
     uint8 lByte=g_arMIDI2ym2149Tone[noteNb].lowbyte;
     uint16 period=g_arMIDI2ym2149Tone[noteNb].period;
	  
     
     switch(channel){
       
      case 1:{
	
       if(bMidiOutput==TRUE){
	note_on(channel,noteNb,127);	//output on channel 9, max velocity
       }
       
       if(bYmOutput==TRUE){
	ch[CH_A].oscFreq=lByte;
	ch[CH_A].oscStepSize=hByte;
	ymDoSound( ch,4 , period,128,CH_A);
       }
      }break;
      case 2:{
	
	 if(bMidiOutput==TRUE){
	    note_on(channel,noteNb,127);	//output on channel 9, max velocity
	 }
	  
	 
	 if(bYmOutput==TRUE){
	    ch[CH_B].oscFreq=lByte;
	    ch[CH_B].oscStepSize=hByte;
	    ymDoSound( ch,4 , period,128,CH_B);
	 }
	
      }break;
      case 3:{
	
	if(bMidiOutput==TRUE){
	  note_on(channel,noteNb,127);	//output on channel 9, max velocity
	}

	 if(bYmOutput==TRUE){
	  ch[CH_C].oscFreq=lByte;
	  ch[CH_C].oscStepSize=hByte;
	  ymDoSound( ch,4 , period,128,CH_C);
	 }
	 
      }break;
      default:{
	
	if(bMidiOutput==TRUE){
	  note_on(channel,noteNb,127);		//output on channel 9, max velocity
	  note_on(channel-1,noteNb,127);	//output on channel 9, max velocity
	  note_on(channel-2,noteNb,127);	//output on channel 9, max velocity
	}
	
	 if(bYmOutput==TRUE){
	  ch[CH_A].oscFreq=lByte;
	  ch[CH_A].oscStepSize=hByte;
	  ch[CH_B].oscFreq=lByte;
	  ch[CH_B].oscStepSize=hByte;
	  ch[CH_C].oscFreq=lByte;
	  ch[CH_C].oscStepSize=hByte;
	 ymDoSound( ch,4 , period,128,CH_ALL);
	}
	
      }break;
     }

}
