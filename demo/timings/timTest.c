
/**  Copyright 2007-2012 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

/////////////////////////////////////////////////////////////////// timing test program
// program reads delta times/notes from table and outputs sound through ym2149/midi out
// with adjustable tempo
///////////////////////////////////////////////////////////////////////////////////////
 
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

sCurrentSequenceState g_CurrentState;

BOOL handleTempoChange;

#ifndef PORTABLE
extern void turnOffKeyclick(void);
extern void customSeqReplay(void);

extern volatile BOOL midiOutputEnabled;
extern volatile BOOL ymOutputEnabled;
#else
BOOL midiOutputEnabled;
BOOL ymOutputEnabled;
void turnOffKeyclick(void){}
void customSeqReplay(void){};
#endif

extern void playNote(U8 noteNb, BOOL bMidiOutput, BOOL bYmOutput);

BOOL isEOT(sEvent *pEvent){
  if(pEvent->delta==0&&pEvent->note==0&&pEvent->tempo==0)
    return TRUE;
  else 
    return FALSE;
}

// plays sample sequence 
int initSequence(sEvent *ch1,sEvent *ch2,sEvent *ch3, sCurrentSequenceState *pInitialState){
U8 mode,data;
  handleTempoChange=FALSE;
  
  pInitialState->tracks[0]->seqPtr=ch1;	
  pInitialState->tracks[0]->state.bIsActive=TRUE;
  pInitialState->tracks[0]->seqPosIdx=0;
  
  pInitialState->tracks[1]->seqPtr=ch2;	
  pInitialState->tracks[1]->state.bIsActive=FALSE;
  pInitialState->tracks[1]->seqPosIdx=0;
  
  pInitialState->tracks[2]->seqPtr=ch3;	
  pInitialState->tracks[2]->state.bIsActive=FALSE;
  pInitialState->tracks[2]->seqPosIdx=0;  

  pInitialState->state=PS_STOPPED;
  pInitialState->currentPPQN=DEFAULT_PPQN;
  pInitialState->currentTempo=DEFAULT_MPQN;
  pInitialState->currentBPM=DEFAULT_BPM;
  pInitialState->defaultPlayMode=S_PLAY_LOOP;
  
  pInitialState->timeElapsedFrac=0;
  pInitialState->timeStep=am_calculateTimeStep(DEFAULT_BPM, DEFAULT_PPQN, SEQUENCER_UPDATE_HZ);

#ifndef PORTABLE
  getMFPTimerSettings(SEQUENCER_UPDATE_HZ,&mode,&data);
  
  //install replay routine 
  installReplayRout(mode, data, (void *)customSeqReplay);
#endif

  return 0;
}

void updateSequenceStep(){
BOOL endOfSequence;

g_CurrentState.timeElapsedFrac += g_CurrentState.timeStep;
U32 TimeAdd = g_CurrentState.timeElapsedFrac >> 16;
g_CurrentState.timeElapsedFrac &= 0xffff;

  //check sequence state if paused do nothing
  if(g_CurrentState.state==PS_PAUSED) return;
  
  //check sequence state if stopped reset position on all tracks
  //and reset tempo to default
  
  if(g_CurrentState.state==PS_STOPPED){
      //repeat for each track
    for (int i=0;i<3;i++){
      g_CurrentState.tracks[i]->seqPosIdx=0;
    }
    //reset tempo to default
    g_CurrentState.currentPPQN=DEFAULT_PPQN;
    g_CurrentState.currentTempo=DEFAULT_MPQN;
    g_CurrentState.currentBPM=DEFAULT_BPM;
    g_CurrentState.timeElapsedFrac=0;
    g_CurrentState.timeStep=am_calculateTimeStep(DEFAULT_BPM, DEFAULT_PPQN, SEQUENCER_UPDATE_HZ);
    return;
  }
  
   
  //repeat for each track
  for (int i=0;i<3;i++){
    
    //for each active track
      U32 count=g_CurrentState.tracks[i]->seqPosIdx;
      sEvent *pEvent=&(g_CurrentState.tracks[i]->seqPtr[count]);
      
      g_CurrentState.tracks[i]->timeElapsedInt+=TimeAdd;
      
      while((!isEOT(pEvent))&&pEvent->delta <= g_CurrentState.tracks[i]->timeElapsedInt ){
	  endOfSequence=FALSE;
	  g_CurrentState.tracks[i]->timeElapsedInt -= pEvent->delta;
	  
	  if(g_CurrentState.tracks[i]->state.bIsActive==TRUE){
	    playNote(pEvent->note,midiOutputEnabled,ymOutputEnabled);
	  }
	  ++count;
	  pEvent=&(g_CurrentState.tracks[i]->seqPtr[count]);
      }
      
      //check for end of sequence
      if(isEOT(pEvent)){
	endOfSequence=TRUE;
      }
  
    g_CurrentState.tracks[i]->seqPosIdx=count;
  }
  
  //check if we have end of sequence
  //on all tracks
  if(endOfSequence==TRUE){
    if(g_CurrentState.defaultPlayMode==S_PLAY_ONCE){
      //reset set state to stopped
      //reset song position on all tracks
      g_CurrentState.state=PS_STOPPED;
    }else if(g_CurrentState.defaultPlayMode==S_PLAY_LOOP){
      g_CurrentState.state=PS_PLAYING;
      g_CurrentState.currentPPQN=DEFAULT_PPQN;
      g_CurrentState.currentTempo=DEFAULT_MPQN;
      g_CurrentState.currentBPM=DEFAULT_BPM;
      g_CurrentState.timeElapsedFrac=0;
      g_CurrentState.timeStep=am_calculateTimeStep(DEFAULT_BPM, DEFAULT_PPQN, SEQUENCER_UPDATE_HZ);
    }
  }
  
  if(handleTempoChange==TRUE){
    g_CurrentState.currentBPM=60000000/g_CurrentState.currentTempo;
    g_CurrentState.timeStep=am_calculateTimeStep(g_CurrentState.currentBPM, g_CurrentState.currentPPQN, SEQUENCER_UPDATE_HZ);
    handleTempoChange=FALSE;
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
#define TEMPO_STEP 10000

int main(void){
  ymChannelData ch[3];
  U32 iCurrentStep=TEMPO_STEP;
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

  //prepare sequence
  initSequence(&testSequenceChannel1[0],&testSequenceChannel2[0],&testSequenceChannel3[0],&g_CurrentState);
  
#ifndef PORTABLE
  /* Install our asm ikbd handler */
  Supexec(IkbdInstall);

  amMemSet(Ikbd_keyboard, KEY_UNDEFINED, sizeof(Ikbd_keyboard));
  Ikbd_mousex = Ikbd_mousey = Ikbd_mouseb = Ikbd_joystick = 0;
  
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

 	    U32 tempo=g_CurrentState.currentTempo;
 	    
 	    if(tempo!=0){
 	      if(tempo<=50000&&tempo>5000){
 		iCurrentStep=5000;
 	      }
 	      else if(tempo<=5000){
 		iCurrentStep=100;
 	      }
 	      else 
		iCurrentStep=TEMPO_STEP;
 	      
	      g_CurrentState.currentTempo=tempo-iCurrentStep;
	      handleTempoChange=TRUE;
	      
 	      printf("Current tempo: %u [ms]\n",(unsigned int)g_CurrentState.currentTempo);
 	    }
	  }break;
	  case SC_ARROW_DOWN:{
	   U32 tempo=g_CurrentState.currentTempo;
 	    if(tempo<800000){
 	      if(tempo<50000){
 	       iCurrentStep=5000;
	      }else 
		iCurrentStep=TEMPO_STEP;
 	    
 	      g_CurrentState.currentTempo=tempo+iCurrentStep;
	      handleTempoChange=TRUE;
 	      printf("Current tempo: %u [ms]",(unsigned int)g_CurrentState.currentTempo);
	    }
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
	    }else if(g_CurrentState.state==S_PLAY_LOOP){
	      iFormerState=g_CurrentState.state;
	      g_CurrentState.state=PS_PAUSED;
	      am_allNotesOff(16);
	      ymSoundOff();
	    }else if(g_CurrentState.state==S_PLAY_RANDOM){
	      iFormerState=g_CurrentState.state;
	      g_CurrentState.state=PS_PAUSED;
	      am_allNotesOff(16);
	      ymSoundOff();
	    }else if(g_CurrentState.state==PS_PAUSED){
	      g_CurrentState.state=iFormerState;
	    }
	  }break;
	  case SC_SPACEBAR:{
	    printf("Stop sequence\n");
	    g_CurrentState.state=PS_STOPPED;
	    am_allNotesOff(16);
	    ymSoundOff();
	  }break;
	  
	} //end switch
     
     } // end if
     
    } //end for
    
    
  }//end while

  am_allNotesOff(16);
  ymSoundOff();
  deinstallReplayRout();   

  /* Uninstall our asm handler */
  Supexec(IkbdUninstall);
#else  
#warning Mainloop not implemented!! TODO!  
#endif
  /* clean up, free internal library buffers etc..*/
  am_deinit();
   
 return 0;
}


