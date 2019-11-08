
/**  Copyright 2007-2014 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#ifndef __MFP_H__
#define __MFP_H__

#include "c_vars.h"

#define MFP_STOP	0b00000000  /* Timer stop */
#define MFP_DIV4	0b00000001  /* div 4 */
#define MFP_DIV10	0b00000010  /* div 10 */
#define MFP_DIV16	0b00000011  /* div 16 */
#define MFP_DIV50	0b00000100  /* div 50 */
#define MFP_DIV64	0b00000101  /* div 64 */
#define MFP_DIV100	0b00000110  /* div 100 */
#define MFP_DIV200	0b00000111  /* div 200 */
#define MFP_EC		0b00001000  /* event count mode (TA/TB)  */ 
#define MFP_DEL4	0b00001001  /* delay 4 */
#define MFP_DEL10	0b00001010  /* delay 10 */
#define MFP_DEL16	0b00001011  /* delay 16 */
#define MFP_DEL50	0b00001100  /* delay 50 */
#define MFP_DEL64	0b00001101  /* delay 64 */
#define MFP_DEL100	0b00001110  /* delay 100 */
#define MFP_DEL200	0b00001111  /* delay 200 */

// timer type on which update will be executed
typedef enum{
  MFP_TiA = 0,
  MFP_TiB = 1,
  MFP_TiC = 2
} eTimerType;

/** installs update sequence, single/multitrack variant hooked to selected timer type */
extern void installReplayRout(const uint8 mode, const uint8 data, const bool isMultitrack, const eTimerType updateHandlerType);

/** deinstalls sequence replay routine installed with installReplayRout()  */
extern void deinstallReplayRout(void);

/** installs sequence replay routine (hooked to timer B atm) [to remove] */
extern void installReplayRoutGeneric(uint8 mode,uint8 data,VOIDFUNCPTR func);

/** deinstalls sequence replay routine (hooked to timer B atm) */
extern void deinstallReplayRoutGeneric(void);

/* ***** */
/* calculates settings for MFP timers for given frequency of tick */
static const uint8 prescales[8]= { 0, 4, 10, 16, 50, 64, 100, 200 };

/** Utility function returns MFP mode and data settings for MFP.
*   @param freq - desired frequency
*   @param mode - pointer to unsigned long int for MFP mode of operation value
*   @param data - pointer to unsigned long int for MFP data value
*/

static inline void getMFPTimerSettings(const uint32 freq, uint8 *mode, uint8 *data){
static uint8 presc=0;
static uint32 temp=0;


if( *mode == MFP_STOP ) {
    *data=(uint8)0;
    return;
}

if(((freq<96) && (freq>=48))) {
  *mode=MFP_DIV200; 		/* divide by 200	*/
  presc=prescales[*mode];
  temp=presc*freq;
  *data=(uint8)((uint32)2457600/temp);
  return;
}

if( ((freq<150)  && (freq>=96))  ) {
  *mode = MFP_DIV100;		/* divide by 100	*/
  presc = prescales[*mode];
  temp = presc*freq;
  *data = (uint8)((uint32)2457600/temp);
  return;
}

if( ((freq<600)  && (freq>=192)) ) {
  *mode = MFP_DIV50;		/* divide by 50 	*/
  presc = prescales[*mode];
  temp = presc*freq;
  *data = (uint8)((uint32)2457600/temp);
  return;
}

if( ((freq<192)  && (freq>=150)) ) {
  *mode = MFP_DIV64;		/* divide by 64 	*/
  presc = prescales[*mode];
  temp = presc*freq;
  *data = (uint8)((uint32)2457600/temp);
  return;
}

if( ((freq<960)  && (freq>=600)) ) {
  *mode = MFP_DIV16;		/* divide by 16 	*/
  presc = prescales[*mode];
  temp = presc*freq;
  *data = (uint8)((uint32)2457600/temp);
  return;
}

if( ((freq<2400) && (freq>=960)) ) {
  *mode = MFP_DIV10;		/* divide by 10 	*/
  presc = prescales[*mode];
  temp = presc*freq;
  *data = (uint8)((uint32)2457600/temp);
  return;
}

if( ( (freq<=614400) && (freq>=2400)) ) {
  *mode = MFP_DIV4;		/* divide by 4  */
  presc = prescales[*mode];
  temp = presc*freq;
  *data = (uint8)((uint32)2457600/temp);
  return;
}
 
 return;
}


#endif
