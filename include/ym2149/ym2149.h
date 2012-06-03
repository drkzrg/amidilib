
/**  Copyright 2007-2010 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#ifndef _YM2149_H_
#define _YM2149_H_

#include "c_vars.h"

#ifndef PORTABLE
#include <osbind.h>
#endif

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

// envelopes
#define ENV_1 0b1001  
#define ENV_2 0b1101 
#define ENV_3 0b1011 
#define ENV_4 0b1111 
#define ENV_5 0b1000 
#define ENV_6 0b1100 
#define ENV_7 0b1010
#define ENV_8 0b1110

typedef struct  {
  U16 period;
  U8 highbyte;
  U8 lowbyte;
}__attribute__((packed)) ymData;

typedef struct{
 U8 amp;	
 U8 oscFreq;		// if 4th bit is set, current envelope is used
 U8 oscStepSize;	// period lenght, pitch
 U8 noiseEnable;	// 0-disabled, 1-enabled
 U8 toneEnable;		// 0-disabled, 1-enabled
}__attribute__((packed)) ymChannelData;

enum{
  CH_A=0,
  CH_B=1,
  CH_C=2
};

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

void ymDoSound(ymChannelData ch[3],U8 envelope, U16 envPeriod,U8 noiseGenPeriod);
void setYm2149(ymChannelData ch[3],int noteIdx,U8 currentEnvelopeIdx, U8 noisegenPeriod);
void ymSoundOff();

static const U8 envelopeArray[8]={ENV_1,ENV_2,ENV_3,ENV_4,ENV_5,ENV_6,ENV_7,ENV_8 };

#endif

