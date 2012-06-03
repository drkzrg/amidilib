
/**  Copyright 2007-2012 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

//////////////////////////////////////////////////// timing test program
// program reads delta times/notes from table and outputs sound through ym2149/midi out
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

#include "sampleSequence.h"

#ifndef PORTABLE
extern void turnOffKeyclick(void);
extern void installYMReplayRout(U8 mode,U8 data, sCurrentSequenceState *pPtr);
extern void deinstallYMReplayRout();

extern volatile U8 tbData,tbMode;
extern volatile BOOL midiOutputEnabled;
extern volatile BOOL ymOutputEnabled;

#else
BOOL midiOutputEnabled;
BOOL ymOutputEnabled;
void turnOffKeyclick(void){}

void installYMReplayRout(U8 mode,U8 data, sCurrentSequenceState *pPtr){
#warning TODO!
}
void deinstallYMReplayRout(){
#warning TODO! 
}
#endif

void playNote(U8 noteNb, BOOL bMidiOutput, BOOL bYmOutput);

BOOL isEOT(sEvent *pEvent){
  if(pEvent->delta==0&&pEvent->note==0&&pEvent->tempo==0)
    return TRUE;
  else 
    return FALSE;
}

// plays sample sequence 
int initSequence(const sEvent *testSequence[3], sCurrentSequenceState *pInitialState){
U8 mode,data; 
  pInitialState->state=PS_STOPPED;			//track state
  pInitialState->currentPPQN=DEFAULT_PPQN;
  pInitialState->currentTempo=DEFAULT_MPQN;
  pInitialState->currentBPM=DEFAULT_TEMPO;
  pInitialState->timeElapsedFrac=0;
  pInitialState->timeStep=am_calculateTimeStep(DEFAULT_TEMPO, DEFAULT_PPQN, SEQUENCER_UPDATE_HZ);
  
  pInitialState->tracks[0]->seqPtr=testSequence[0];	
  pInitialState->tracks[0]->state.bIsActive=TRUE;
  pInitialState->tracks[1]->seqPtr=testSequence[1];	
  pInitialState->tracks[1]->state.bIsActive=TRUE;
  pInitialState->tracks[2]->seqPtr=testSequence[2];	
  pInitialState->tracks[2]->state.bIsActive=TRUE;
  
  getMFPTimerSettings(SEQUENCER_UPDATE_HZ,&mode,&data);
  
  //install replay routine 
  installYMReplayRout(mode, data, pInitialState);
  return 0;
}

sCurrentSequenceState g_CurrentState;

void updateSequenceStep(){
   g_CurrentState.timeElapsedFrac += g_CurrentState.timeStep;
   U32 TimeAdd = g_CurrentState.timeElapsedFrac >> 16;
   g_CurrentState.timeElapsedFrac &= 0xffff;

  for (int i=0;i<3;i++){
    //for each active track
    if(g_CurrentState.tracks[i]->seqPtr!=0&&g_CurrentState.tracks[i]->state.bIsActive==TRUE){
      
      sEvent *pEvent=g_CurrentState.tracks[i]->seqPtr;
      g_CurrentState.tracks[i]->timeElapsedInt+=TimeAdd;
      
      while(pEvent!=0&&(!isEOT(pEvent))&&pEvent->delta <= g_CurrentState.tracks[i]->timeElapsedInt ){
	  g_CurrentState.tracks[i]->timeElapsedInt -= pEvent->delta;
	  playNote(pEvent->note,midiOutputEnabled,ymOutputEnabled);
	  pEvent++;
      }
      //update track counter
      g_CurrentState.tracks[i]->seqPtr=pEvent;
    }  
    
  }
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




 
int main(void){
 

  ymChannelData ch[3];

  g_CurrentState.currentTempo=DEFAULT_TEMPO;
  g_CurrentState.currentPPQN=DEFAULT_PPQN;
  
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
  
  //prepare sequence
  const sEvent *sequences[3]={0};
  sequences[0]=testSequenceChannel1;
  sequences[1]=testSequenceChannel2;
  sequences[2]=testSequenceChannel3;
  
  initSequence(sequences,&g_CurrentState);
  
  //enter main loop
  while(bQuit==FALSE){
  //printf("%ld, %ld \n",counter,g_CurrentState.currentIdx);
    
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
// 	    U32 tempo=g_CurrentState.currentTempo;
// 	    if(tempo<800000){
// 	    
// 	      if(tempo<50000){
// 	       iCurrentStep=5000;
// 	    }else 
// 	      iCurrentStep=TEMPO_STEP;
// 	    
// 	     g_CurrentState.currentTempo=tempo+iCurrentStep;
// 	     U32 freq=(U32)(g_CurrentState.currentTempo/g_CurrentState.currentPPQN);
// 	     
// 	    printf("Current tempo: %u [ms](freq %u),\ntimer mode: %u, count:%u\n",(unsigned int)g_CurrentState.currentTempo,(unsigned int)freq,(unsigned int)tbMode,(unsigned int)tbData);}
// 	  
	  }break;
	  case SC_ARROW_DOWN:{
// 	    U32 tempo=g_CurrentState.currentTempo;
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
// 	      g_CurrentState.currentTempo=tempo-iCurrentStep;
// 	      U32 freq=(U32)(g_CurrentState.currentTempo/g_CurrentState.currentPPQN);
// 	      printf("Current tempo: %u [ms](freq %u),\ntimer mode: %u, count:%u\n",(unsigned int)g_CurrentState.currentTempo,(unsigned int)freq,(unsigned int)tbMode,(unsigned int)tbData);
// 	    }
// 	    
	  }break;
	  
	  case SC_I:{
	    printHelpScreen();
	  }break;
	  case SC_M:{
	    // toggle play mode PLAY ONCE / LOOP
	    
	    if(g_CurrentState.state==S_PLAY_LOOP){
	      printf("Play sequence once.\n");
	      g_CurrentState.state=S_PLAY_ONCE;
	    }
	    else if(g_CurrentState.state==S_PLAY_ONCE){
	      printf("Play sequence in loop.\n");
	      g_CurrentState.state=S_PLAY_LOOP;
	    }else{
	      if(g_CurrentState.defaultPlayMode==S_PLAY_LOOP){
		printf("Play sequence once.\n");
		g_CurrentState.defaultPlayMode=S_PLAY_ONCE;
	      }
	      else if(g_CurrentState.defaultPlayMode==S_PLAY_ONCE){
		printf("Play sequence in loop.\n");
		g_CurrentState.defaultPlayMode=S_PLAY_LOOP;
	      }
	    
	    }
	    
	  }break;
	  
	  case SC_P:{
	    printf("Pause/Resume sequence\n");
	    static U32 iFormerState;
	    if(g_CurrentState.state==PS_STOPPED){
	      g_CurrentState.state=g_CurrentState.defaultPlayMode;
	    }else if(g_CurrentState.state==S_PLAY_ONCE){
	      iFormerState=g_CurrentState.state; 
	      g_CurrentState.state=PS_PAUSED;
	      am_allNotesOff(16);
	      ymSoundOff();
	    }
	    else if(g_CurrentState.state==S_PLAY_LOOP){
	      iFormerState=g_CurrentState.state;
	      g_CurrentState.state=PS_PAUSED;
	      am_allNotesOff(16);
	      ymSoundOff();
	    }
	    else if(g_CurrentState.state==S_PLAY_RANDOM){
	      iFormerState=g_CurrentState.state;
	      g_CurrentState.state=PS_PAUSED;
	      am_allNotesOff(16);
	      ymSoundOff();
	    }
	    else if(g_CurrentState.state==PS_PAUSED){
	      g_CurrentState.state=iFormerState;
	      
	    }
	  }break;
	  case SC_SPACEBAR:{
	    printf("Stop sequence\n");
	    g_CurrentState.state=PS_STOPPED;
	    am_allNotesOff(16);
	    ymSoundOff();
	  }break;
	  
	}
     
     }
     
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


