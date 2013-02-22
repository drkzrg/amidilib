
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
#include "ym2149/ym2149.h"
#include "timing/miditim.h"
#include "sampleSequence.h"

#define TEMPO_STEP 10000

#ifdef PORTABLE
volatile BOOL midiOutputEnabled;
volatile BOOL ymOutputEnabled;
volatile BOOL handleTempoChange;

void turnOffKeyclick(void){}
void customSeqReplay(void){};
#else

#include <osbind.h>
#include "input/ikbd.h"
#include "timing/mfp.h"

extern void customSeqReplay(void);
extern void playNote(U8 noteNb, BOOL bMidiOutput, BOOL bYmOutput);
extern void amMidiSendIKBD();

volatile BOOL handleTempoChange;
BOOL midiOutputEnabled;
BOOL ymOutputEnabled;
#endif


// functions
void onTogglePlayMode(sCurrentSequenceState *pSeqPtr);
void onTempoUp(sCurrentSequenceState *pSeqPtr);
void onTempoDown(sCurrentSequenceState *pSeqPtr);
void onToggleMidiEnable();
void onToggleYmEnable();
void onTogglePlayPauseSequence(sCurrentSequenceState *pSeqPtr);
void onStopSequence(sCurrentSequenceState *pSeqPtr);
BOOL isEndSeq(sEvent *pEvent);

// plays sample sequence 
int initSequence(sEvent *ch1,sEvent *ch2,sEvent *ch3, sCurrentSequenceState *pSeqPtr);
void updateSequenceStep(); 
void onEndSeq(); //end sequence handler
void printHelpScreen();

sCurrentSequenceState g_CurrentState; //current sequence

int main(void){
  ymChannelData ch[3];
  midiOutputEnabled=FALSE;
  ymOutputEnabled=TRUE;
  BOOL bQuit=FALSE;

  //set up ym2149 sound
  /////////////////////////////////////
  ch[CH_A].amp=16;
  ch[CH_A].oscFreq=0;
  ch[CH_A].oscStepSize=0;  
  ch[CH_A].toneEnable=1;
  ch[CH_A].noiseEnable=0;
  
  ch[CH_B].amp=16;
  ch[CH_B].oscFreq=0;
  ch[CH_B].oscStepSize=0;
  ch[CH_B].toneEnable=1;
  ch[CH_B].noiseEnable=0;
  
  ch[CH_C].amp=16;
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
  initSequence(&testSequenceChannel1[0],&testSequenceChannel2[1],&testSequenceChannel3[2],&g_CurrentState);
 
  
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
	    onToggleMidiEnable();
 	  }break;
	  case SC_2:{
	    onToggleYmEnable();
	  }break;
	  case SC_ARROW_UP:{
	    onTempoUp(&g_CurrentState);
	  }break;
	  case SC_ARROW_DOWN:{
	    onTempoDown(&g_CurrentState);
	  }break;
	  case SC_I:{
	    printHelpScreen();
	  }break;
	  case SC_M:{
	    onTogglePlayMode(&g_CurrentState);
	  }break;
	  case SC_P:{
	    onTogglePlayPauseSequence(&g_CurrentState);
	  }break;
	  case SC_SPACEBAR:{
	    onStopSequence(&g_CurrentState);
	    initSequence(&testSequenceChannel1[0],&testSequenceChannel2[1],&testSequenceChannel3[2],&g_CurrentState);
	  }break;
	  
	} //end switch
     
     } // end if
     
    } //end for
    
    
  }//end while

  am_allNotesOff(16);
  amMidiSendIKBD();
  
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

void onTogglePlayMode(sCurrentSequenceState *pState){
// toggle play mode PLAY ONCE / LOOP
  if(pState->state==S_PLAY_LOOP){
      printf("Play sequence once.\n");
      pState->state=S_PLAY_ONCE;
    }
    else if(g_CurrentState.state==S_PLAY_ONCE){
      printf("Play sequence in loop.\n");
      pState->state=S_PLAY_LOOP;
    }else{
	if(pState->playMode==S_PLAY_LOOP){
	 printf("Play sequence once.\n");
	 pState->playMode=S_PLAY_ONCE;
	}
	else if(g_CurrentState.playMode==S_PLAY_ONCE){
	  printf("Play sequence in loop.\n");
	  pState->playMode=S_PLAY_LOOP;
	}
    }
}

void onTempoUp(sCurrentSequenceState *pSeqPtr){
U32 iCurrentStep;
  if(g_CurrentState.state==PS_STOPPED) return;
  
  if(pSeqPtr->currentTempo<0) pSeqPtr->currentTempo=0;

  if(pSeqPtr->currentTempo!=0){
    if(pSeqPtr->currentTempo<=50000&&pSeqPtr->currentTempo>5000){
      iCurrentStep=5000;
    }
    else if(pSeqPtr->currentTempo<=5000){
      iCurrentStep=100;
    }
    else 
      iCurrentStep=TEMPO_STEP;
 
    if(!(pSeqPtr->currentTempo-iCurrentStep<=0))
      pSeqPtr->currentTempo-=iCurrentStep;
    
    handleTempoChange=TRUE;    
  }
  
  printf("Current tempo: %ld [ms]\n",pSeqPtr->currentTempo);
}

void onTempoDown(sCurrentSequenceState *pSeqPtr){
U32 iCurrentStep;

if(g_CurrentState.state==PS_STOPPED) return;

  if(pSeqPtr->currentTempo<50000){
    iCurrentStep=5000;
  }else 
    iCurrentStep=TEMPO_STEP;  
   
  pSeqPtr->currentTempo+=iCurrentStep;
  handleTempoChange=TRUE;
 
  printf("Current tempo: %ld [ms]\n",pSeqPtr->currentTempo);
}

void onToggleMidiEnable(){
  printf("MIDI output ");
  if(midiOutputEnabled==TRUE){
    midiOutputEnabled=FALSE;
    am_allNotesOff(16);
    printf("disabled.\n");
   }else{
    midiOutputEnabled=TRUE;
    printf("enabled.\n");
   }
}

void onToggleYmEnable(){
  printf("ym2149 output ");
  if(ymOutputEnabled==TRUE){
    ymOutputEnabled=FALSE;
    ymSoundOff();
    printf("disabled.\n");
  }else{
    ymOutputEnabled=TRUE;
    printf("enabled.\n");
  }
}

void onTogglePlayPauseSequence(sCurrentSequenceState *pSeqPtr){

printf("Pause/Resume sequence\n");
  
  if(pSeqPtr->state==PS_STOPPED){
      pSeqPtr->state=PS_PLAYING;
  }else if(pSeqPtr->state==PS_PLAYING){
      pSeqPtr->state=PS_PAUSED;
  }else if(pSeqPtr->state==PS_PAUSED){
      pSeqPtr->state=PS_PLAYING;
  }
}

void onStopSequence(sCurrentSequenceState *pSeqPtr){
  printf("Stop sequence\n");
  pSeqPtr->state=PS_STOPPED;
  am_allNotesOff(16);
  ymSoundOff();
}

BOOL isEndSeq(sEvent *pEvent){
  if(pEvent->delta==0&&pEvent->note==0)
    return TRUE;
  else 
    return FALSE;
}

void INLINE printHelpScreen(){
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

// plays sample sequence 
int initSequence(sEvent *ch1,sEvent *ch2,sEvent *ch3, sCurrentSequenceState *pSeqPtr){
  static BOOL bPlayModeInit=FALSE;
  U8 mode,data;
  handleTempoChange=FALSE;
  VOIDFUNCPTR replayRout=customSeqReplay;
 
  pSeqPtr->tracks[0].seqPtr=ch1;	
  pSeqPtr->tracks[0].state.bIsActive=TRUE;
  pSeqPtr->tracks[0].seqPosIdx=0;
  
  pSeqPtr->tracks[1].seqPtr=ch2;	
  pSeqPtr->tracks[1].state.bIsActive=TRUE;
  pSeqPtr->tracks[1].seqPosIdx=0;
  
  pSeqPtr->tracks[2].seqPtr=ch3;	
  pSeqPtr->tracks[2].state.bIsActive=TRUE;
  pSeqPtr->tracks[2].seqPosIdx=0;  

  pSeqPtr->state=PS_STOPPED;
  pSeqPtr->currentPPQN=DEFAULT_PPQN;
  pSeqPtr->currentTempo=DEFAULT_MPQN;
  pSeqPtr->currentBPM=DEFAULT_BPM;
 
  pSeqPtr->timeElapsedFrac=0;
  pSeqPtr->timeStep=am_calculateTimeStep((U16)DEFAULT_BPM, (U16)DEFAULT_PPQN, (U16)SEQUENCER_UPDATE_HZ);
  
   if(bPlayModeInit==FALSE){
     //init but only once, user can switch this option during runtime
      bPlayModeInit=TRUE;
      g_CurrentState.playMode=S_PLAY_LOOP; 
    }
  
#ifndef PORTABLE
  getMFPTimerSettings(SEQUENCER_UPDATE_HZ,&mode,&data);

  //install replay routine 
  installReplayRout(mode, data, replayRout);
#else
#warning Not implemented!  
#endif

  return 0;
}



void updateSequenceStep(){
BOOL endOfSequence;
static BOOL bStopped=FALSE;
  
  //check sequence state if paused do nothing
  if(g_CurrentState.state==PS_PAUSED) {
    if(midiOutputEnabled==TRUE) am_allNotesOff(16);
    if(ymOutputEnabled==TRUE) ymSoundOff();
    return;
  }
  
  if(g_CurrentState.state==PS_PLAYING) bStopped=FALSE;
  //check sequence state if stopped reset position on all tracks
  //and reset tempo to default, but only once
  
  if(g_CurrentState.state==PS_STOPPED&&bStopped==FALSE){
    bStopped=TRUE;
    //repeat for each track
    for (int i=0;i<3;i++){
      g_CurrentState.tracks[i].seqPosIdx=0;
    }
    //reset tempo to default
    g_CurrentState.currentPPQN=DEFAULT_PPQN;
    g_CurrentState.currentTempo=DEFAULT_MPQN;
    g_CurrentState.currentBPM=DEFAULT_BPM;
    g_CurrentState.timeElapsedFrac=0;
    
    g_CurrentState.timeStep=am_calculateTimeStep(DEFAULT_BPM, DEFAULT_PPQN, SEQUENCER_UPDATE_HZ);
    return;
  }
  else if(g_CurrentState.state==PS_STOPPED){
    return;
  }
  
  if(handleTempoChange==TRUE){
    g_CurrentState.currentBPM=60000000/g_CurrentState.currentTempo;
    g_CurrentState.timeStep=am_calculateTimeStep(g_CurrentState.currentBPM, g_CurrentState.currentPPQN, SEQUENCER_UPDATE_HZ);
    //amTrace("\nSet new timestep:%d\n",g_CurrentState.timeStep);
    handleTempoChange=FALSE;
  }
  
   g_CurrentState.timeElapsedFrac += g_CurrentState.timeStep;
   U32 TimeAdd = g_CurrentState.timeElapsedFrac >> 16;
   g_CurrentState.timeElapsedFrac &= 0xffff;
   
  //repeat for each track
  for (int i=0;i<3;i++){
    //for each active track
      U32 count=g_CurrentState.tracks[i].seqPosIdx;
      sEvent *pEvent=&(g_CurrentState.tracks[i].seqPtr[count]);
      
      g_CurrentState.tracks[i].timeElapsedInt+=TimeAdd;
      
      while((isEndSeq(pEvent)==FALSE)&&pEvent->delta <= g_CurrentState.tracks[i].timeElapsedInt ){
	  endOfSequence=FALSE;
	  g_CurrentState.tracks[i].timeElapsedInt -= pEvent->delta;
	  
	  if(g_CurrentState.tracks[i].state.bIsActive==TRUE){
	    playNote(pEvent->note,midiOutputEnabled,ymOutputEnabled);
	  }
	  ++count;
	  pEvent=&(g_CurrentState.tracks[i].seqPtr[count]);
      }
      
      //check for end of sequence
      if(isEndSeq(pEvent)){
	endOfSequence=TRUE;
      }
  
    g_CurrentState.tracks[i].seqPosIdx=count;
  }
  
  //check if we have end of sequence
  //on all tracks
  if(endOfSequence==TRUE){
    onEndSeq();
    endOfSequence=FALSE;
  }
}

void onEndSeq(){

  if(g_CurrentState.playMode==S_PLAY_ONCE){
      //reset set state to stopped
      //reset song position on all tracks
      g_CurrentState.state=PS_STOPPED;
      
    }else if(g_CurrentState.playMode==S_PLAY_LOOP){
      g_CurrentState.state=PS_PLAYING;
    }
    
    if(midiOutputEnabled==TRUE) am_allNotesOff(16);
    if(ymOutputEnabled==TRUE) ymSoundOff();
  
    g_CurrentState.currentPPQN=DEFAULT_PPQN;
    g_CurrentState.currentTempo=DEFAULT_MPQN;
    g_CurrentState.currentBPM=DEFAULT_BPM;
    g_CurrentState.timeElapsedFrac=0;
    g_CurrentState.timeStep=am_calculateTimeStep(DEFAULT_BPM, DEFAULT_PPQN, SEQUENCER_UPDATE_HZ); 
     
    for (int i=0;i<3;i++){
      g_CurrentState.tracks[i].seqPosIdx=0;
    }  
  
}


