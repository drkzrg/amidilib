#include <math.h>

#include "include/timing/mfp.h"

#ifndef PORTABLE
/* calculates settings for MFP timers for given frequency of tick */
static const U8 prescales[8]= { 0, 4, 10, 16, 50, 64, 100, 200 };

void getMFPTimerSettings(U32 freq,U8 *mode,U8 *data){
U8 cntrl,count;
cntrl=0;

if( freq<=614400 && freq>=2400 ) {
  cntrl=MFP_DIV4;		/* divide by 4  */
  U8 presc=prescales[cntrl];
  U32 temp=presc*freq;
  count=(U8)((U32)2457600/temp);
  
  *mode=cntrl;
  *data=count;
  
  return;	 
}
	
if( freq<2400 && freq>=960 ) {
  cntrl=MFP_DIV10;		/* divide by 10 	*/
  U8 presc=prescales[cntrl];
  U32 temp=presc*freq;
  count=(U8)((U32)2457600/temp);
  *mode=cntrl;
  *data=count;
  
  return;
}

if( freq<960  && freq>=600 ) {
  cntrl=MFP_DIV16;		/* divide by 16 	*/
  U8 presc=prescales[cntrl];
  U32 temp=presc*freq;
  count=(U8)((U32)2457600/temp);
   *mode=cntrl;
  *data=count;
  
  return;
}

if( freq<600  && freq>=192 ) {
  cntrl=MFP_DIV50;		/* divide by 50 	*/
  U8 presc=prescales[cntrl];
  U32 temp=presc*freq;
  count=(U8)((U32)2457600/temp);
  *mode=cntrl;
  *data=count;
 
  return;
}

if( freq<192  && freq>=150 ) {
  cntrl=MFP_DIV64;		/* divide by 64 	*/
  U8 presc=prescales[cntrl];
  U32 temp=presc*freq;
  count=(U8)((U32)2457600/temp);
  *mode=cntrl;
  *data=count;
  return;
}

if( freq<150  && freq>=96  ) {
  cntrl=MFP_DIV100;		/* divide by 100	*/
  U8 presc=prescales[cntrl];
  U32 temp=presc*freq;
  count=(U8)((U32)2457600/temp);
  *mode=cntrl;
  *data=count;
  return;
}
		
if( freq<96&&freq>=48) {
  cntrl=MFP_DIV200; 		/* divide by 200	*/
  U8 presc=prescales[cntrl];
  U32 temp=presc*freq;
  count=(U8)((U32)2457600/temp);
  *mode=cntrl;
  *data=count;
  
  return;
}
	
 if( cntrl==0 ) {
  count=0;    
  *mode=0;
  *data=count;
 
 }
return;
}

#endif