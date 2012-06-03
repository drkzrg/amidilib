
/**  Copyright 2007-2012 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

//////////////////////////////////////////////////// timing test program
// program reads delta times/notes from table and outputs sound through ym2149
// with adjustable tempo
/////////////////////////////////////////////////////////////////////////
 
#include <stdio.h>
#include <ctype.h> 

#include "amidilib.h"
#include "ym2149.h" 

#ifndef PORTABLE
#include <osbind.h>
#include "input/ikbd.h"
#include "scancode.h"
#include "timing/mfp.h"
#endif

typedef struct{
  U32 delta;
  U32 tempo;	// 0 == stop
  U8 note;	// 0-127 range
  U8 dummy;	// just fill in
} sEvent; 

typedef struct{
  BOOL bIsActive;
  U8 volume;
}sTrackState; 

/////////////////////////////////////////////////
//check if we are on the end of test sequence
typedef struct{
  sTrackState state;
  U32 timeElapsedInt;
  const sEvent *seqPtr;	
} sTrack;

typedef struct{
  U32 currentTempo;	//quaternote duration in ms, 500ms default
  U32 currentPPQN;	//pulses per quater note
  U32 currentBPM;	//beats per minute (60 000000 / currentTempo)
  U32 timeElapsedFrac; //sequence elapsed time
  U32 timeStep; 	//sequence elapsed time
  sTrack *tracks[3];	//one per ym channel
  U32 state;		// 0=STOP, 1-PLAYING, 2-PAUSED
} sCurrentSequenceState;



#ifndef PORTABLE
extern void turnOffKeyclick(void);
extern void installYMReplayRout(U8 mode,U8 data,volatile sCurrentSequenceState *pPtr);
extern void deinstallYMReplayRout();

extern volatile U8 tbData,tbMode;
extern volatile BOOL midiOutputEnabled;
extern volatile BOOL ymOutputEnabled;
#else
BOOL midiOutputEnabled;
BOOL ymOutputEnabled;
void turnOffKeyclick(void){}

void installYMReplayRout(U8 mode,U8 data,volatile sCurrentSequenceState *pPtr){
#warning TODO!
}
void deinstallYMReplayRout(){
#warning TODO! 
}
#endif

void playNote(U8 noteNb, BOOL bMidiOutput, BOOL bYmOutput);

sCurrentSequenceState currentState;
volatile extern U32 counter;
extern U32 defaultPlayMode;

static U32 iCurrentStep;

// plays sample sequence 
int playSampleSequence(const sEvent *testSequence[3], sCurrentSequenceState *pInitialState){
U8 mode,data; 
  pInitialState->state=PS_STOPPED;			//track state
  pInitialState->currentPPQN=DEFAULT_PPQN;
  pInitialState->currentTempo=DEFAULT_MPQN;
  pInitialState->currentBPM=DEFAULT_TEMPO;
  pInitialState->timeElapsedFrac=0;
  pInitialState->timeStep=am_calculateTimeStep(pInitialState->currentBPM, pInitialState->currentPPQN, SEQUENCER_UPDATE_HZ);
  
  pInitialState->tracks[0]->seqPtr=testSequence[0];	//ptr to sequence
  pInitialState->tracks[0]->state.bIsActive=TRUE;
  pInitialState->tracks[1]->seqPtr=testSequence[1];	//ptr to sequence
  pInitialState->tracks[1]->state.bIsActive=TRUE;
  pInitialState->tracks[2]->seqPtr=testSequence[2];	//ptr to sequence
  pInitialState->tracks[2]->state.bIsActive=TRUE;
  
  getMFPTimerSettings(SEQUENCER_UPDATE_HZ,&mode,&data);
  
  //install replay routine 
  installYMReplayRout(mode, data, pInitialState);
  return 0;
}

BOOL isEOT(sEvent *pSeqPtr){

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
  printf("(c) Nokturnal 2012\n");
  printf("================================================\n");
}

extern U8 envelopeArray[8];

// output, test sequence for channel 1 
static const sEvent testSequenceChannel1[]={
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
static const sEvent testSequenceChannel2[]={
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
static const sEvent testSequenceChannel3[]={
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
  ymChannelData ch[3];

  currentState.currentTempo=DEFAULT_TEMPO;
  currentState.currentPPQN=DEFAULT_PPQN;
  
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
  
  printHelpScreen();
  turnOffKeyclick();

#ifndef PORTABLE
  /* Install our asm ikbd handler */
  Supexec(IkbdInstall);

  amMemSet(Ikbd_keyboard, KEY_UNDEFINED, sizeof(Ikbd_keyboard));
  Ikbd_mousex = Ikbd_mousey = Ikbd_mouseb = Ikbd_joystick = 0;
  U8 mode,data;
  
  U32 freq=currentState.currentTempo/currentState.currentPPQN;			//
  
  getMFPTimerSettings(freq,&mode,&data);
  
  //prepare sequence
  const sEvent *sequences[3]={0};
  sequences[0]=testSequenceChannel1;
  sequences[1]=testSequenceChannel2;
  sequences[2]=testSequenceChannel3;
  
  playSampleSequence(sequences,&currentState);
  
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
// 	    U32 tempo=currentState.currentTempo;
// 	    if(tempo<800000){
// 	    
// 	      if(tempo<50000){
// 	       iCurrentStep=5000;
// 	    }else 
// 	      iCurrentStep=TEMPO_STEP;
// 	    
// 	     currentState.currentTempo=tempo+iCurrentStep;
// 	     U32 freq=(U32)(currentState.currentTempo/currentState.currentPPQN);
// 	     
// 	    printf("Current tempo: %u [ms](freq %u),\ntimer mode: %u, count:%u\n",(unsigned int)currentState.currentTempo,(unsigned int)freq,(unsigned int)tbMode,(unsigned int)tbData);}
// 	  
	  }break;
	  case SC_ARROW_DOWN:{
// 	    U32 tempo=currentState.currentTempo;
// 	    
// 	    if(tempo!=0){
// 	       
// 	      if(tempo<=50000&&tempo>5000){
// 		iCurrentStep=5000;
// 	      }
// 	      else if(tempo<=5000){
// 		iCurrentStep=100;
// 	      }
// 	      else iCurrentStep=TEMPO_STEP;
// 	      currentState.currentTempo=tempo-iCurrentStep;
// 	      U32 freq=(U32)(currentState.currentTempo/currentState.currentPPQN);
// 	      printf("Current tempo: %u [ms](freq %u),\ntimer mode: %u, count:%u\n",(unsigned int)currentState.currentTempo,(unsigned int)freq,(unsigned int)tbMode,(unsigned int)tbData);
// 	    }
// 	    
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


