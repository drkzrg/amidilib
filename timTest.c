
/**  Copyright 2007-2010 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

/////////////////////////////// timing test program
// program reads delta times from table and outputs sound through ym2149
// with given, adjustable tempo
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
  U32 state;		// 0=STOP, 1-PLAYING, 2-PAUSED
} sCurrentSequenceState;

enum{
 STOP=0, 
 PLAY_ONCE, 
 PLAY_LOOP,
 PLAY_RANDOM,
 PAUSED
} eSeqState;

sCurrentSequenceState currentState;
ymChannelData ch[3];
volatile extern U32 counter;
extern void installReplayRout(sCurrentSequenceState *pPtr);
extern void deinstallReplayRout();

//plays given note and outputs it to midi/ym2149
void playNote(U8 noteNb, BOOL bMidiOutput, BOOL bYmOutput){
  
  if(bMidiOutput==TRUE){
    note_on(noteNb,1,127);	//output on channel 2, max velocity
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
    
    ymDoSound( ch, 1, 127,127);
    
  }

}

// plays sample sequence 
int playSampleSequence(sSequence *testSequenceChannel1, U32 tempo, sCurrentSequenceState *pInitialState){
  pInitialState->currentIdx=0;
  pInitialState->currentTempo=tempo;
  pInitialState->state=STOP;
  pInitialState->seqPtr=testSequenceChannel1;
  
  //install replay routine 
  installReplayRout(pInitialState);
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

BOOL midiOutputEnabled;
BOOL ymOutputEnabled;
  
int main(void){
  U32 defaultTempo=500;
  
  // midi initial settings
  U8 currentChannel=1;
  U8 currentVelocity=127;
  U8 currentPN=1;
  U8 currentBankSelect=0;
  midiOutputEnabled=FALSE;
  ymOutputEnabled=TRUE;
  
  BOOL bPause=FALSE;
  BOOL bQuit=FALSE;

  //set up ym2149 sound
  /////////////////////////////////////
  ch[CH_A].amp=15;
  ch[CH_A].oscFreq=0;
  ch[CH_A].oscStepSize=0;  
  ch[CH_A].toneEnable=1;
  ch[CH_A].noiseEnable=0;
  
  ch[CH_B].amp=15;
  ch[CH_B].oscFreq=0;
  ch[CH_B].oscStepSize=0;
  ch[CH_B].toneEnable=1;
  ch[CH_B].noiseEnable=0;
  
  ch[CH_C].amp=15;
  ch[CH_C].oscFreq=0;
  ch[CH_C].oscStepSize=0;
  ch[CH_C].toneEnable=1;
  ch[CH_C].noiseEnable=0;
  ////////////////////////////////////////
  
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
  printf("%ld\n",counter);
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
	    currentState.currentTempo++;
	    printf("Current tempo: %ld [ms]\n",currentState.currentTempo);
	  }break;
	  case SC_ARROW_DOWN:{
	    if(currentState.currentTempo!=0){
	      currentState.currentTempo--;
	    }
	    printf("Current tempo: %ld\n",currentState.currentTempo);
	  }break;
	  case SC_Q:{
	    playNote(33,midiOutputEnabled,ymOutputEnabled);
	  }break;
	  case SC_I:{
	    printHelpScreen();
	  }break;
	  case SC_P:{
	    printf("Pause/Resume sequence\n");
	    static U32 iFormerState;
	    if(currentState.state==STOP){
	      currentState.state=iFormerState;
	    }else if(currentState.state==PLAY_ONCE){
	      iFormerState=currentState.state; 
	      currentState.state=PAUSED;
	    }
	    else if(currentState.state==PLAY_ONCE){
	      iFormerState=currentState.state;
	      currentState.state=PAUSED;
	    }
	    
	  }break;
	  case SC_SPACEBAR:{
	    printf("Stop sequence\n");
	    currentState.state=STOP;
	    deinstallReplayRout();   
	    am_allNotesOff(16);
	    ymSoundOff();
    
	  }break;
	  
	}
     
     }
     
     if (Ikbd_keyboard[i]==KEY_RELEASED) {
	Ikbd_keyboard[i]=KEY_UNDEFINED;
     }
     
     
    }
    
    
  }

  am_allNotesOff(16);
  ymSoundOff();
  deinstallReplayRout();   

  /* Uninstall our asm handler */
  Supexec(IkbdUninstall);

  /* clean up, free internal library buffers etc..*/
  am_deinit();
   
 return 0;
}


