
/**  Copyright 2007-2021 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#ifndef _YM2149_H_
#define _YM2149_H_

#include "vartypes.h"

// ym2149 registers constants
static const uint8 LB_OSC1=0;
static const uint8 HB_OSC1=1;
static const uint8 LB_OSC2=2;
static const uint8 HB_OSC2=3;
static const uint8 LB_OSC3=4;
static const uint8 HB_OSC3=5;
static const uint8 NOISE_GEN=6;
static const uint8 MIXER=7;
static const uint8 AMP_OSC1=8;
static const uint8 AMP_OSC2=9;
static const uint8 AMP_OSC3=10;
static const uint8 LB_ENV_PERIOD=11;
static const uint8 HB_ENV_PERIOD=12;
static const uint8 ENV_SELECT=13;

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
  uint16 period;
  uint8 highbyte;
  uint8 lowbyte;
} ymData;

typedef struct{
 uint8 amp;	
 uint8 oscFreq;		// if 4th bit is set, current envelope is used
 uint8 oscStepSize;	// period lenght, pitch  //TODO: change it to bitfield
 uint8 noiseEnable;	// 0-disabled, 1-enabled
 uint8 toneEnable;		// 0-disabled, 1-enabled
} ymChannelData;

enum{
  CH_A=0,
  CH_B=1,
  CH_C=2,
  CH_ALL
};

void ymDoSound(ymChannelData ch[3], const uint8 envelope, const uint16 envPeriod,const uint8 noiseGenPeriod, const uint8 affectsChannels);
void setYm2149(ymChannelData ch[3],const int noteIdx,const uint8 currentEnvelopeIdx, const uint8 noisegenPeriod);
void ymSoundOff(void);

eYMenvelope getEnvelopeId(const uint8 id);

#endif

