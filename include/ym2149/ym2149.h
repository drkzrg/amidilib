
/**  Copyright 2007-2010 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#ifndef _YM2149_H_
#define _YM2149_H_

#include "c_vars.h"

// ym2149 registers constants
static const U8 LB_OSC1=0;
static const U8 HB_OSC1=1;
static const U8 LB_OSC2=2;
static const U8 HB_OSC2=3;
static const U8 LB_OSC3=4;
static const U8 HB_OSC3=5;
static const U8 NOISE_GEN=6;
static const U8 MIXER=7;
static const U8 AMP_OSC1=8;
static const U8 AMP_OSC2=9;
static const U8 AMP_OSC3=10;
static const U8 LB_ENV_PERIOD=11;
static const U8 HB_ENV_PERIOD=12;
static const U8 ENV_SELECT=13;

// envelopes
typedef enum{
 ENV_1=0b1001,
 ENV_2=0b1101,
 ENV_3=0b1011,
 ENV_4=0b1111,
 ENV_5=0b1000,
 ENV_6=0b1100,
 ENV_7=0b1010,
 ENV_8=0b1110,
 ENV_MAX
} eYMenvelope;

typedef struct  {
  U16 period;
  U8 highbyte;
  U8 lowbyte;
} ymData;

typedef struct{
 U8 amp;	
 U8 oscFreq;		// if 4th bit is set, current envelope is used
 U8 oscStepSize;	// period lenght, pitch  //TODO: change it to bitfield
 U8 noiseEnable;	// 0-disabled, 1-enabled
 U8 toneEnable;		// 0-disabled, 1-enabled
} ymChannelData;

enum{
  CH_A=0,
  CH_B=1,
  CH_C=2,
  CH_ALL
};

void ymDoSound(ymChannelData ch[3],U8 envelope, U16 envPeriod,U8 noiseGenPeriod,U8 affectsChannels);
void setYm2149(ymChannelData ch[3],int noteIdx,U8 currentEnvelopeIdx, U8 noisegenPeriod);
void ymSoundOff();

eYMenvelope getEnvelopeId(const U8 id);

#endif

