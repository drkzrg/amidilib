
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
#include "mfp.h"

extern void turnOffKeyclick(void);

typedef struct{
  U32 delta;
  U32 tempo;	// 0 == stop
  U8 note;	// 0-127 range
  U8 dummy;	// just fill in
} sSequence; 

// output, test sequence for channel 1 
static const sSequence testSequenceChannel1[]={
  {0L,500,56,0xAD},
  {32L,500,127,0xAD},
  {32L,500,110,0xAD},
  {32L,500,127,0xAD},
  {32L,500,110,0xAD},
  {32L,500,127,0xAD},
  {32L,500,110,0xAD},
  {32L,500,127,0xAD},
  {32L,500,110,0xAD},
  {32L,500,127,0xAD},
  {32L,500,110,0xAD},
  {0,0,0,0xAD}
};

// output test sequence for channel 2
static const sSequence testSequenceChannel2[]={
  {0L,500L,36,0xAD},
  {64L,500L,37,0xAD},
  {128L,500L,36,0xAD},
  {64L,500L,37,0xAD},
  {128L,500L,36,0xAD},
  {64L,500L,37,0xAD},
  {128L,500L,36,0xAD},
  {64L,500L,37,0xAD},
  {0L,500L,40,0xAD},
  {0L,500L,41,0xAD},
  {0L,500L,42,0xAD},
  {0L,500L,43,0xAD},
  {1L,500L,65,0xAD},
  {2L,500L,66,0xAD},
  {3L,500L,65,0xAD},
  {4L,500L,66,0xAD},
  {5L,500L,65,0xAD},
  {6L,500L,66,0xAD},
  {7L,500L,65,0xAD},
  {8L,500L,66,0xAD},
  {9L,500L,65,0xAD},
  {10L,500L,66,0xAD},
  {15L,500L,65,0xAD},
  {20L,500L,66,0xAD},
  {40L,500L,65,0xAD},
  {80L,500L,66,0xAD},
  {160L,500L,65,0xAD},
  {320L,500L,66,0xAD},
  {0L,0L,0,0,0xAD}
};

/////////////////////////////////////////////////
//check if we are on the end of test sequence

typedef struct{
  volatile U32 currentTempo;	//quaternote duration in ms, 500ms default
  volatile U32 currentPPQN;	//pulses per quater note
  volatile U32 currentIdx;	//current position in table
  volatile sSequence *seqPtr;	//sequence ptr
  volatile U32 state;		// 0=STOP, 1-PLAYING, 2-PAUSED
} sCurrentSequenceState;

enum{
 STOP=0, 
 PLAY_ONCE=1, 
 PLAY_LOOP=2,
 PLAY_RANDOM=3,
 PAUSED=4
} eSeqState;

volatile sCurrentSequenceState currentState;
volatile extern U32 counter;

ymChannelData ch[3];

extern void installReplayRout(U8 mode,U8 data,sCurrentSequenceState *pPtr);
extern void deinstallReplayRout();

//plays given note and outputs it to midi/ym2149
void playNote(U8 noteNb, BOOL bMidiOutput, BOOL bYmOutput){

  if(bMidiOutput==TRUE){
    note_on(9,noteNb,127);	//output on channel 2, max velocity
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
    
    ymDoSound( ch, 1, period,0);
    
  }

}

// plays sample sequence 
int playSampleSequence(sSequence *testSequenceChannel1, U32 tempo, sCurrentSequenceState *pInitialState){
  pInitialState->currentIdx=0;
  pInitialState->currentTempo=tempo;
  pInitialState->state=PLAY_LOOP;
  pInitialState->seqPtr=testSequenceChannel1;
  
  //install replay routine 
  installReplayRout(MFP_DIV10, 59, pInitialState);
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
  
  printf("[spacebar] - turn off all sounds / stop sequence \n");
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
  U8 currentPN=127;
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
  //printf("%ld, %ld \n",counter,currentState.currentIdx);
    
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
	      am_allNotesOff(16);
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
	      ymSoundOff();
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
	    printf("Current tempo: %ld[ms]\n",currentState.currentTempo);
	  }break;
	  case SC_I:{
	    printHelpScreen();
	  }break;
	  case SC_P:{
	    printf("Pause/Resume sequence\n");
	    static U32 iFormerState=PLAY_ONCE;
	    if(currentState.state==STOP){
	      currentState.state=iFormerState;
	    }else if(currentState.state==PLAY_ONCE){
	      iFormerState=currentState.state; 
	      currentState.state=PAUSED;
	      am_allNotesOff(16);
	      ymSoundOff();
	    }
	    else if(currentState.state==PLAY_LOOP){
	      iFormerState=currentState.state;
	      currentState.state=PAUSED;
	      am_allNotesOff(16);
	      ymSoundOff();
	    }
	    else if(currentState.state==PLAY_RANDOM){
	      iFormerState=currentState.state;
	      currentState.state=PAUSED;
	      am_allNotesOff(16);
	      ymSoundOff();
	    }
	    else if(currentState.state==PAUSED){
	      currentState.state=iFormerState;
	      
	    }
	  }break;
	  case SC_SPACEBAR:{
	    printf("Stop sequence\n");
	    currentState.state=STOP;
	    am_allNotesOff(16);
	    ymSoundOff();
	  }break;
	  
	}
     
     }
     
     /*if (Ikbd_keyboard[i]==KEY_RELEASED) {
	Ikbd_keyboard[i]=KEY_UNDEFINED;
     }*/
     
     
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


