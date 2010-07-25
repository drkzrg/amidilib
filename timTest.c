
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
#include "amidilib.h"
#include "ikbd.h"
#include "ym2149.h" 
#include "scancode.h"

extern void turnOffKeyclick(void);

typedef struct{
  U32 delta;
  U8 note;	// 0-127 range
  U32 tempo;	// 0 == stop
} sSequence; 

// output, test sequence for channel 1 
static const sSequence testSequenceChannel1[]={
  {0,56,500},
  {32,127,500},
  {32,110,500},
  {0,0,0}
};

// output test sequence for channel 2
static const sSequence testSequenceChannel2[]={
  {0,56,500},
  {32,127,500},
  {32,110,500},
  {0,111,500},
  {0,0,0}
};

/////////////////////////////////////////////////
//check if we are on the end of test sequence

typedef struct  {

  U32 currentTempo;
  U32 currentIdx;	//current position in table
  sSequence *seqPtr;	//sequence ptr
  U32 state;	// 0=STOP, 1-PLAYING, 2-PAUSED
} sCurrentSequenceState;

enum{
 STOP=0, 
 PLAYING, 
 PAUSED
} eSeqState;

sCurrentSequenceState currentState;

int playSampleSequence(sSequence *testSequenceChannel1, U32 tempo, sCurrentSequenceState *pInitialState){
  pInitialState->currentIdx=0;
  pInitialState->currentTempo=tempo;
  pInitialState->state=STOP;
  pInitialState->seqPtr=testSequenceChannel1;
  
  //install replay routine 
  
  return 0;
}

BOOL isEOT(sSequence *pSeqPtr){

  if((pSeqPtr->delta==0&&pSeqPtr->note==0&&pSeqPtr->tempo==0)) return TRUE;
   else return FALSE;
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

extern U8 envelopeArray[8];
static const U8 KEY_PRESSED = 0xff;
static const U8 KEY_UNDEFINED=0x80;
static const U8 KEY_RELEASED=0x00;


int main(void){
  U32 defaultTempo=500;
  
  // midi initial settings
  U8 currentChannel=1;
  U8 currentVelocity=127;
  U8 currentPN=1;
  U8 currentBankSelect=0;
  
  BOOL midiOutputEnabled=FALSE;
  BOOL ymOutputEnabled=TRUE;
  BOOL bPause=FALSE;
  BOOL bQuit=FALSE;

  /* init library */
  U32 iError=am_init();
 
  //set current channel as 1, default is 0 in external module
  control_change(0x00, currentChannel, currentBankSelect,0x00);
  program_change(currentChannel, currentPN);
  
  turnOffKeyclick();

  /* Install our asm ikbd handler */
  Supexec(IkbdInstall);
  
  printHelpScreen();

  memset(Ikbd_keyboard, KEY_UNDEFINED, sizeof(Ikbd_keyboard));
  Ikbd_mousex = Ikbd_mousey = Ikbd_mouseb = Ikbd_joystick = 0;

  //prepare sequence
  playSampleSequence(testSequenceChannel2,defaultTempo, &currentState);
  
  //enter main loop
  while(bQuit==FALSE){
  
    for (int i=0; i<128; i++) {
     
     if (Ikbd_keyboard[i]==KEY_PRESSED) {
	Ikbd_keyboard[i]=KEY_UNDEFINED;
	
	switch(i){
	  case SC_ESC:{
	    bQuit=TRUE;
	 }break;
	  case SC_1:{
	    printf("MIDI output ");
	    if(midiOutputEnabled==TRUE){
	      midiOutputEnabled=FALSE;
	      printf("disabled.\n");
	    }else{
	      midiOutputEnabled=TRUE;
	      printf("enabled.\n");
	    }
 	  }break;
	  case SC_2:{
	    printf("ym2149 output ");
	    if(ymOutputEnabled==TRUE){
	      ymOutputEnabled=FALSE;
	      printf("disabled.\n");
	    }else{
	      ymOutputEnabled=TRUE;
	      printf("enabled.\n");
	    }
	  }break;
	  case SC_ARROW_UP:{
	    defaultTempo++;
	    printf("Current tempo: %ld [ms]\n",defaultTempo);
	  }break;
	  case SC_ARROW_DOWN:{
	    if(defaultTempo!=0){
	      defaultTempo--;
	    }
	    printf("Current tempo: %ld\n",defaultTempo);
	  }break;
	  case SC_I:{
	    printHelpScreen();
	  }break;
	  case SC_P:{
	    printf("Pause/Resume sequence\n");
	  }break;
	  case SC_SPACEBAR:{
	    printf("Stop sequence\n");
	  }break;
	  
	}
     
     }
     
     if (Ikbd_keyboard[i]==KEY_RELEASED) {
	Ikbd_keyboard[i]=KEY_UNDEFINED;
     }
     
     
    }
    
    
  }

  /* Uninstall our asm handler */
  Supexec(IkbdUninstall);

  /* clean up, free internal library buffers etc..*/
  am_deinit();
   
 return 0;
}


