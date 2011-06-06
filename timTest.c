
/**  Copyright 2007-2010 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

//////////////////////////////////////////////////// timing test program
// program reads delta times from table and outputs sound through ym2149
// with given, adjustable tempo
/////////////////////////////////////////////////////////////////////////
 
#include <stdio.h>
#include <ctype.h> 

#include "amidilib.h"
#include "ym2149.h" 

#ifndef PORTABLE
#include <osbind.h>
#include "ikbd.h"
#include "scancode.h"
#include "mfp.h"
#endif

#define TEMPO_STEP 50000	// tempo change step in ms


typedef struct{
  U32 delta;
  U32 tempo;	// 0 == stop
  U8 note;	// 0-127 range
  U8 dummy;	// just fill in
} sSequence; 


/////////////////////////////////////////////////
//check if we are on the end of test sequence

typedef struct{
  volatile U32 currentTempo;	//quaternote duration in ms, 500ms default
  volatile U32 currentPPQN;	//pulses per quater note
  volatile U32 currentIdx;	//current position in table
  volatile const sSequence *seqPtr;	//sequence ptr
  volatile U32 state;		// 0=STOP, 1-PLAYING, 2-PAUSED
} sCurrentSequenceState;


#ifndef PORTABLE
extern void turnOffKeyclick(void);
extern void installYMReplayRout(U8 mode,U8 data,volatile sCurrentSequenceState *pPtr);
extern void deinstallYMReplayRout();
extern volatile U8 tbData,tbMode;
#else
void turnOffKeyclick(void){
#warning TODO!
}

void installYMReplayRout(U8 mode,U8 data,volatile sCurrentSequenceState *pPtr){
#warning TODO!
}
void deinstallYMReplayRout(){
#warning TODO! 
}
#endif

void playNote(U8 noteNb, BOOL bMidiOutput, BOOL bYmOutput);

volatile sCurrentSequenceState currentState;
volatile extern U32 counter;
extern U32 defaultPlayMode;

ymChannelData ch[3];
static U32 iCurrentStep;
//plays given note and outputs it to midi/ym2149
void playNote(U8 noteNb, BOOL bMidiOutput, BOOL bYmOutput){

  if(bMidiOutput==TRUE){
    note_on(9,noteNb,127);	//output on channel 9, max velocity
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
    
    ymDoSound( ch,4 , period,128);
    
  }

}

// plays sample sequence 
int playSampleSequence(const sSequence *testSequenceChannel1, U32 mode,U32 data, volatile sCurrentSequenceState *pInitialState){
  pInitialState->currentIdx=0;			//initial position
  pInitialState->state=PS_STOPPED;			//track state
  pInitialState->seqPtr=(const sSequence *)testSequenceChannel1;	//ptr to sequence
  
  //install replay routine 96 ticks per 500ms interval 
  installYMReplayRout(mode, data, pInitialState);
  return 0;
}

BOOL isEOT(sSequence *pSeqPtr){

  if((pSeqPtr->delta==0&&pSeqPtr->note==0&&pSeqPtr->tempo==0)) return TRUE;
   else return FALSE;
}

void printHelpScreen(){
  printf("===============================================\n");
  printf("/|\\ delta timing and sound output test..\n");
  printf("[arrow up/ arrow down] - change tempo \n\t500 ms/PQN and 96PPQN\n");
  printf("[1/2] - enable/disable midi out/ym2149 output \n");
  printf("[m] - toggle [PLAY ONCE/LOOP] sequence replay mode \n");
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
  {0L,0L,0,0xAD}
};

// output test sequence for channel 2
static const sSequence testSequenceChannel3[]={
  {10L,500L,65,0xAD},
  {10L,500L,66,0xAD},
  {10L,500L,65,0xAD},
  {10L,500L,66,0xAD},
  {10L,500L,65,0xAD},
  {10L,500L,66,0xAD},
  {10L,500L,65,0xAD},
  {10L,500L,66,0xAD},
  {10L,500L,65,0xAD},
  {10L,500L,66,0xAD},
  {10L,500L,65,0xAD},
  {10L,500L,66,0xAD},
  {10L,500L,65,0xAD},
  {10L,500L,66,0xAD},
  {10L,500L,65,0xAD},
  {10L,500L,66,0xAD},
  {10L,500L,65,0xAD},
  {10L,500L,66,0xAD},
  {10L,500L,65,0xAD},
  {10L,500L,66,0xAD},
  {10L,500L,65,0xAD},
  {10L,500L,66,0xAD},
  {10L,500L,65,0xAD},
  {10L,500L,66,0xAD},
  {0L,0L,0,0xAD}
};



int main(void){
  U32 defaultTempo=60000000/120;
  iCurrentStep=TEMPO_STEP;
  currentState.currentTempo=defaultTempo;
  currentState.currentPPQN=96;
  
  // midi initial settings
  U8 currentChannel=1;
  U8 currentPN=127;
  U8 currentBankSelect=0;
  
  midiOutputEnabled=FALSE;
  ymOutputEnabled=TRUE;
  
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
 
  if(iError!=1) return -1;
  
  //set current channel as 1, default is 0 in external module
  control_change(0x00, currentChannel, currentBankSelect,0x00);
  program_change(currentChannel, currentPN);
  printHelpScreen();
  turnOffKeyclick();

#ifndef PORTABLE
  /* Install our asm ikbd handler */
  Supexec(IkbdInstall);

  amMemSet(Ikbd_keyboard, KEY_UNDEFINED, sizeof(Ikbd_keyboard));
  Ikbd_mousex = Ikbd_mousey = Ikbd_mouseb = Ikbd_joystick = 0;
  U32 mode,data;
  
  U32 freq=currentState.currentTempo/currentState.currentPPQN;			//
  
  getMFPTimerSettings(freq,&mode,&data);
  
  //prepare sequence
  playSampleSequence(testSequenceChannel3,mode,data, &currentState);
  
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
	    U32 tempo=currentState.currentTempo;
	    if(tempo<800000){
	    
	      if(tempo<50000){
	       iCurrentStep=5000;
	    }else 
	      iCurrentStep=TEMPO_STEP;
	    
	     currentState.currentTempo=tempo+iCurrentStep;
	     U32 freq=(U32)(currentState.currentTempo/currentState.currentPPQN);
	     
	    printf("Current tempo: %u [ms](freq %u),\ntimer mode: %u, count:%u\n",(unsigned int)currentState.currentTempo,(unsigned int)freq,(unsigned int)tbMode,(unsigned int)tbData);}
	  
	  }break;
	  case SC_ARROW_DOWN:{
	    U32 tempo=currentState.currentTempo;
	    
	    if(tempo!=0){
	      
	      if(tempo<=50000&&tempo>5000){
		iCurrentStep=5000;
	      }
	      else if(tempo<=5000){
		iCurrentStep=100;
	      }
	      else iCurrentStep=TEMPO_STEP;
	      currentState.currentTempo=tempo-iCurrentStep;
	      U32 freq=(U32)(currentState.currentTempo/currentState.currentPPQN);
	      printf("Current tempo: %u [ms](freq %u),\ntimer mode: %u, count:%u\n",(unsigned int)currentState.currentTempo,(unsigned int)freq,(unsigned int)tbMode,(unsigned int)tbData);
	    }
	    
	  }break;
	  
	  case SC_I:{
	    printHelpScreen();
	  }break;
	  case SC_M:{
	    // toggle play mode PLAY ONCE / LOOP
	    
	    if(currentState.state==S_PLAY_LOOP){
	      printf("Play sequence once.\n");
	      currentState.state=S_PLAY_ONCE;
	    }
	    else if(currentState.state==S_PLAY_ONCE){
	      printf("Play sequence in loop.\n");
	      currentState.state=S_PLAY_LOOP;
	    }else{
	      if(defaultPlayMode==S_PLAY_LOOP){
		printf("Play sequence once.\n");
		defaultPlayMode=S_PLAY_ONCE;
	      }
	      else if(defaultPlayMode==S_PLAY_ONCE){
		printf("Play sequence in loop.\n");
		defaultPlayMode=S_PLAY_LOOP;
	      }
	    
	    }
	    
	  }break;
	  
	  case SC_P:{
	    printf("Pause/Resume sequence\n");
	    static U32 iFormerState;
	    if(currentState.state==PS_STOPPED){
	      currentState.state=defaultPlayMode;
	    }else if(currentState.state==S_PLAY_ONCE){
	      iFormerState=currentState.state; 
	      currentState.state=PS_PAUSED;
	      am_allNotesOff(16);
	      ymSoundOff();
	    }
	    else if(currentState.state==S_PLAY_LOOP){
	      iFormerState=currentState.state;
	      currentState.state=PS_PAUSED;
	      am_allNotesOff(16);
	      ymSoundOff();
	    }
	    else if(currentState.state==S_PLAY_RANDOM){
	      iFormerState=currentState.state;
	      currentState.state=PS_PAUSED;
	      am_allNotesOff(16);
	      ymSoundOff();
	    }
	    else if(currentState.state==PS_PAUSED){
	      currentState.state=iFormerState;
	      
	    }
	  }break;
	  case SC_SPACEBAR:{
	    printf("Stop sequence\n");
	    currentState.state=PS_STOPPED;
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
  deinstallYMReplayRout();   

  /* Uninstall our asm handler */
  Supexec(IkbdUninstall);
#else  
#warning Mainloop not implemented!! TODO!  
#endif
  /* clean up, free internal library buffers etc..*/
  am_deinit();
   
 return 0;
}


