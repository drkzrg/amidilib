
/**  Copyright 2007-2019 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

/////////////////////////////////////////////////////////////////// timing test program
// program reads delta times/notes from table and outputs sound through ym2149/midi out
// with adjustable tempo
///////////////////////////////////////////////////////////////////////////////////////
 
#include <stdio.h>
#include <ctype.h> 
#include <stdio.h>

#include "amidilib.h"

#include "ym2149/ym2149.h"
#include "timing/miditim.h"

#include "sampleSequence.h"


#define TEMPO_STEP 10000UL

#include "input/ikbd.h"
#include "timing/mfp.h"

#include <mint/osbind.h>

extern void customSeqReplay(void);
extern void playNote(uint8 channel,uint8 noteNb, bool bMidiOutput, bool bYmOutput);


volatile bool handleTempoChange;
bool midiOutputEnabled;
bool ymOutputEnabled;

// functions
void onTogglePlayMode(sCurrentSequenceState *pSeqPtr);
void onTempoUp(sCurrentSequenceState *pSeqPtr);
void onTempoDown(sCurrentSequenceState *pSeqPtr);
void onToggleMidiEnable();
void onToggleYmEnable();
void onTogglePlayPauseSequence(sCurrentSequenceState *pSeqPtr);
void onStopSequence(sCurrentSequenceState *pSeqPtr);
bool isEndSeq(sEvent *pEvent);

// plays sample sequence 
int initSampleSequence(sEvent *ch1,sEvent *ch2,sEvent *ch3, sCurrentSequenceState *pSeqPtr);
void updateSequenceStep(); 
void onEndSeq(); //end sequence handler
void printHelpScreen();

sCurrentSequenceState g_CurrentState; //current sequence

int main(void){
  ymChannelData ch[3];
  midiOutputEnabled=FALSE;
  ymOutputEnabled=TRUE;
  bool bQuit=FALSE;

  //set up ym2149 sound
  /////////////////////////////////////
  ch[CH_A].amp=16;
  ch[CH_A].oscFreq=getEnvelopeId(0);
  ch[CH_A].oscStepSize=15;  
  ch[CH_A].toneEnable=1;
  ch[CH_A].noiseEnable=0;
  
  ch[CH_B].amp=16;
  ch[CH_B].oscFreq=getEnvelopeId(0);
  ch[CH_B].oscStepSize=8;
  ch[CH_B].toneEnable=1;
  ch[CH_B].noiseEnable=0;
  
  ch[CH_C].amp=16;
  ch[CH_C].oscFreq=getEnvelopeId(0);
  ch[CH_C].oscStepSize=6;
  ch[CH_C].toneEnable=1;
  ch[CH_C].noiseEnable=0;
  ////////////////////////////////////////
  
  /* init library */
  uint32 iError=am_init();
 
  if(iError!=1) return -1;
  
  printHelpScreen();
  turnOffKeyclick();

  //prepare sequence
  sEvent *ch1=getTestSequenceChannel(0);
  sEvent *ch2=getTestSequenceChannel(1);
  sEvent *ch3=getTestSequenceChannel(2);

  initSampleSequence(ch1,ch2,ch3,&g_CurrentState);
  
  /* Install our asm ikbd handler */
  Supexec(IkbdInstall);

  amMemSet(Ikbd_keyboard, KEY_UNDEFINED, sizeof(Ikbd_keyboard));
  Ikbd_mousex = Ikbd_mousey = Ikbd_mouseb = Ikbd_joystick = 0;
  
  //enter main loop
  while(bQuit==FALSE){
    
  for (uint16 i=0; i<128; ++i) {
     
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
        sEvent *ch1=getTestSequenceChannel(0);
        sEvent *ch2=getTestSequenceChannel(1);
        sEvent *ch3=getTestSequenceChannel(2);

        initSequence(ch1,ch2,ch3,&g_CurrentState);
	  }break;
	  
	} //end switch
     
     } // end if
     
    } //end for
    
    
  }//end while

  am_allNotesOff(16);
  Supexec(flushMidiSendBuffer);
  
  ymSoundOff();
  deinstallReplayRoutGeneric();

  /* Uninstall our asm handler */
  Supexec(IkbdUninstall);

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
uint32 iCurrentStep=0L;
uint32 iCurrentTempo=0L;

  if(handleTempoChange!=FALSE) return;

  iCurrentTempo=pSeqPtr->currentTempo;

  if(pSeqPtr->state==PS_STOPPED) return;
  
  if(iCurrentTempo<=0UL) {
    pSeqPtr->currentTempo=0L;
    return;
  }
    
  if((iCurrentTempo<=50000UL&&iCurrentTempo>5000UL)){
      iCurrentStep=5000UL;
  }else if(iCurrentTempo<=5000UL){
      iCurrentStep=100UL;
  }else{ 
      iCurrentStep=TEMPO_STEP;
  }
  
  if(!((iCurrentTempo-iCurrentStep)<=0UL)){
      iCurrentTempo=iCurrentTempo-iCurrentStep;
      pSeqPtr->currentTempo=iCurrentTempo;
  }

  printf("qn duration: %lu [ms]\n",iCurrentTempo);
  handleTempoChange=TRUE;    
}

void onTempoDown(sCurrentSequenceState *pSeqPtr){
uint32 iCurrentStep=0L;
uint32 iCurrentTempo=0L;

if(handleTempoChange!=FALSE) return;

if(g_CurrentState.state==PS_STOPPED) return;

iCurrentTempo=pSeqPtr->currentTempo;

  if(iCurrentTempo<=50000UL){
    iCurrentStep=5000UL;
  }else if(iCurrentTempo>50000UL){
    iCurrentStep=TEMPO_STEP;  
  } 
  
  iCurrentTempo=iCurrentTempo+iCurrentStep;
  pSeqPtr->currentTempo=iCurrentTempo;
  
  printf("qn duration: %lu [ms]\n",iCurrentTempo);

  handleTempoChange=TRUE;
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
  pSeqPtr->currentBPM=DEFAULT_BPM;
  pSeqPtr->currentTempo=DEFAULT_MPQN;
  pSeqPtr->currentPPQN=DEFAULT_PPQN;
  pSeqPtr->timeElapsedFrac=0UL;

  pSeqPtr->timeStep=am_calculateTimeStep(g_CurrentState.currentBPM, g_CurrentState.currentPPQN, SEQUENCER_UPDATE_HZ);
  
  for (uint16 i=0;i<3;++i){
      pSeqPtr->tracks[i].seqPosIdx=0UL;
      pSeqPtr->tracks[i].timeElapsedInt=0UL;
  }  
  
  am_allNotesOff(16);
  ymSoundOff();
}

bool isEndSeq(sEvent *pEvent){
  if((pEvent->delta==0&&pEvent->note==0))
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
  printf("(c) Nokturnal 2013\n");
  printf("================================================\n");
}

// plays sample sequence 
int initSampleSequence(sEvent *ch1,sEvent *ch2,sEvent *ch3, sCurrentSequenceState *pSeqPtr){
  static bool bPlayModeInit=FALSE;
  uint8 mode=0,data=0;
  handleTempoChange=FALSE;
  VOIDFUNCPTR replayRout=customSeqReplay;
 
  pSeqPtr->tracks[0].seqPtr=ch1;	
  pSeqPtr->tracks[0].state.bIsActive=TRUE;
  pSeqPtr->tracks[0].seqPosIdx=0;
  pSeqPtr->tracks[0].timeElapsedInt=0UL;
  
  pSeqPtr->tracks[1].seqPtr=ch2;	
  pSeqPtr->tracks[1].state.bIsActive=TRUE;
  pSeqPtr->tracks[1].seqPosIdx=0;
  pSeqPtr->tracks[1].timeElapsedInt=0UL;
   
  pSeqPtr->tracks[2].seqPtr=ch3;	
  pSeqPtr->tracks[2].state.bIsActive=TRUE;
  pSeqPtr->tracks[2].seqPosIdx=0;  
  pSeqPtr->tracks[2].timeElapsedInt=0UL;
 
  pSeqPtr->state=PS_STOPPED;
  pSeqPtr->currentPPQN=DEFAULT_PPQN;
  pSeqPtr->currentTempo=DEFAULT_MPQN;
  pSeqPtr->currentBPM=DEFAULT_BPM;
 
  pSeqPtr->timeElapsedFrac=0;
  pSeqPtr->timeStep=am_calculateTimeStep((uint16)DEFAULT_BPM, (uint16)DEFAULT_PPQN, (uint16)SEQUENCER_UPDATE_HZ);
  
   if(bPlayModeInit==FALSE){
     //init but only once, user can switch this option during runtime
      bPlayModeInit=TRUE;
      g_CurrentState.playMode=S_PLAY_LOOP; 
    }
  
  getMFPTimerSettings(SEQUENCER_UPDATE_HZ,&mode,&data);

  //install replay routine 
  installReplayRoutGeneric(mode, data, replayRout);

  return 0;
}



void updateSequenceStep(){
static bool endOfSequence=FALSE;
static bool bStopped=FALSE;
  
  //check sequence state if paused do nothing
  if(g_CurrentState.state==PS_PAUSED) {
    if(midiOutputEnabled==TRUE) am_allNotesOff(16);
    if(ymOutputEnabled==TRUE) ymSoundOff();
    return;
  }
  
  if(g_CurrentState.state==PS_PLAYING) bStopped=FALSE;
  //check sequence state if stopped reset position on all tracks
  //and reset tempo to default, but only once
  
  if((g_CurrentState.state==PS_STOPPED&&bStopped==FALSE)){
    bStopped=TRUE;
    //repeat for each track
    for (uint16 i=0;i<3;++i){
      g_CurrentState.tracks[i].seqPosIdx=0;
      g_CurrentState.tracks[i].timeElapsedInt=0UL;
    }
    //reset tempo to default
    g_CurrentState.currentPPQN=DEFAULT_PPQN;
    g_CurrentState.currentTempo=DEFAULT_MPQN;
    g_CurrentState.currentBPM=DEFAULT_BPM;
    g_CurrentState.timeElapsedFrac=0UL;
    
    g_CurrentState.timeStep=am_calculateTimeStep(DEFAULT_BPM, DEFAULT_PPQN, SEQUENCER_UPDATE_HZ);
    
    if(midiOutputEnabled==TRUE) am_allNotesOff(16);
    if(ymOutputEnabled==TRUE) ymSoundOff();
    
    return;
  }
  else if(g_CurrentState.state==PS_STOPPED&&bStopped==TRUE){
    return;
  }
  
  if(handleTempoChange==TRUE){
    g_CurrentState.currentBPM=60000000/g_CurrentState.currentTempo;
    g_CurrentState.timeStep=am_calculateTimeStep(g_CurrentState.currentBPM, g_CurrentState.currentPPQN, SEQUENCER_UPDATE_HZ);
    //amTrace("\nSet new timestep:%d\n",g_CurrentState.timeStep);
    handleTempoChange=FALSE;
  }
  
   g_CurrentState.timeElapsedFrac += g_CurrentState.timeStep;
   uint32 TimeAdd = g_CurrentState.timeElapsedFrac >> 16;
   g_CurrentState.timeElapsedFrac &= 0xffff;
   
  //repeat for each track
  for (uint16 i=0;i<3;++i){

     //for each active track
      uint32 count=g_CurrentState.tracks[i].seqPosIdx;
      sEvent *pEvent=&(g_CurrentState.tracks[i].seqPtr[count]);
      
      g_CurrentState.tracks[i].timeElapsedInt+=TimeAdd;
      
      while( ((isEndSeq(pEvent)==FALSE)&&pEvent->delta<=g_CurrentState.tracks[i].timeElapsedInt))
      {
        endOfSequence=FALSE;
        g_CurrentState.tracks[i].timeElapsedInt -= pEvent->delta;
	  
        if(g_CurrentState.tracks[i].state.bIsActive==TRUE){
            playNote(i+1,pEvent->note,midiOutputEnabled,ymOutputEnabled);
        }

        ++count;
        pEvent=&(g_CurrentState.tracks[i].seqPtr[count]);
      }
      
      //check for end of sequence
      if(isEndSeq(pEvent)){
        endOfSequence=TRUE;
        playNote(i+1,0,midiOutputEnabled,ymOutputEnabled);
      }else{
        g_CurrentState.tracks[i].seqPosIdx=count;
     }
    
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
      onStopSequence(&g_CurrentState);
      return;
    }else if(g_CurrentState.playMode==S_PLAY_LOOP){
      g_CurrentState.state=PS_PLAYING;  
      
      g_CurrentState.currentPPQN=DEFAULT_PPQN;
      g_CurrentState.currentBPM=60000000/g_CurrentState.currentTempo;  //do not reset current tempo !!!!
      g_CurrentState.timeElapsedFrac=0UL;
      g_CurrentState.timeStep=am_calculateTimeStep(g_CurrentState.currentBPM, DEFAULT_PPQN, SEQUENCER_UPDATE_HZ); 
  
      for (uint16 i=0;i<3;++i){
        g_CurrentState.tracks[i].seqPosIdx=0UL;
        g_CurrentState.tracks[i].timeElapsedInt=0UL;
       }
       
       if(midiOutputEnabled==TRUE) am_allNotesOff(16);
       if(ymOutputEnabled==TRUE) ymSoundOff();
       
         
   }
    
    
}


