
/**  Copyright 2007-2010 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

/////////////////////////////// timing test program
// program reads delta times from table and outputs sound through ym2149
// with given tempo
/////////////////////////////////////////////////////////////////////////
 
#include <stdio.h>
#include <ctype.h> 
#include <osbind.h>
#include "ym2149.h" 
 
void hMidiEvent(){
  //dummy, TODO: remove it  
}

extern void turnOffKeyclick(void);

typedef struct{
  U32 delta;
  U8 note;	// 0-127 range
  U32 tempo;	// 0 == stop
} sSequence; 

///////////////////////////////////////////////

// output, test sequence for channel 1 
static const sSequence testSequenceChannel1[]={
  {32,127,500},
  {32,110,500},
  {0,0,0}
};

// output test sequence for channel 2
static const sSequence testSequenceChannel2[]={
  {32,127,500},
  {32,110,500},
  {0,0,0}
};

/////////////////////////////////////////////////
//check if we are on the end of test sequence

BOOL isEOT(sSequence *pSeqPtr){

if((pSeqPtr->delta==0&&pSeqPtr->note==0&&pSeqPtr->tempo==0)){
  return TRUE;
} else 
  return FALSE;
}

void printHelpScreen(){
  printf("===============================================\n");
  printf("/|\\ delta timing and sound output test..\n");
  printf("[arrow up/ arrow down] - change tempo [500 ms default]\n");
  printf("[1/2] - enable/disable midi out/ym2149 output \n");
  printf("[p] - pause/resume sequence \n");
  printf("[i] - show this help screen \n");
  
  printf("[spacebar] - turn off all sounds / stop \n");
  printf("[Esc] - quit\n");
  printf("(c) Nokturnal 2010\n");
  printf("================================================\n");
}




int main(void){
  U32 defaultTempo=500;
  BOOL midiOutputEnabled=FALSE;
  BOOL ymOutputEnabled=TRUE;
  BOOL bPause=FALSE;
  BOOL bQuit=FALSE;
  
  turnOffKeyclick();
  
  printHelpScreen();

  //enter main loop
  
  while(bQuit==FALSE){
  
  
  }


 return 0;
}


