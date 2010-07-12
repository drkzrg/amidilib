
/**  Copyright 2007-2010 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#ifndef _YM2149_H__
#define _YM2149_H__

#include "include/c_vars.h"
#include <osbind.h>

//ym2149 registers constants
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

typedef struct {
  U16 period;
  U8 highbyte;
  U8 lowbyte;
} ymData;

inline void ymDoSound(U8 hByte,U8 lByte){
    Giaccess(254,MIXER+128);	// sound chip setup
    Giaccess(15,AMP_OSC1+128);
    Giaccess(lByte,LB_OSC1+128);	// set note
    Giaccess(hByte,HB_OSC1+128);
}

inline void ymSoundOff(){
  Giaccess(255,MIXER+128);
}

#endif

