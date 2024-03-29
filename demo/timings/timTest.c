
/**  Copyright 2007-2022 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

/////////////////////////////////////////////////////////////////// timing test program
// program reads delta times/notes from table and outputs sound through ym2149/midi out
// with adjustable tempo
///////////////////////////////////////////////////////////////////////////////////////
 
#include <ctype.h> 

#include "amidilib.h"

#include "ym2149/ym2149.h"
#include "timing/miditim.h"

#if AMIDILIB_USE_LIBC
#include <string.h>
#else
#include "amstring.h"
#endif

#include "core/amprintf.h"

#include "sampleSequence.h"


#define TEMPO_STEP 10000UL

#include "input/ikbd.h"
#include "timing/mfp.h"
#include <mint/osbind.h>

AM_EXTERN void customSeqReplay(void);
AM_EXTERN void playNote(uint8 channel,uint8 noteNb, bool bMidiOutput, bool bYmOutput);
volatile bool handleTempoChange;
bool midiOutputEnabled;
bool ymOutputEnabled;

// functions
void onTogglePlayMode(sCurrentSequenceState *pSeqPtr);
void onTempoUp(sCurrentSequenceState *pSeqPtr);
void onTempoDown(sCurrentSequenceState *pSeqPtr);
void onToggleMidiEnable(void);
void onToggleYmEnable(void);
void onTogglePlayPauseSequence(sCurrentSequenceState *pSeqPtr);
void onStopSequence(sCurrentSequenceState *pSeqPtr);
bool isEndSeq(sEvent *pEvent);

// plays sample sequence 
int initSampleSequence(sEvent *ch1,sEvent *ch2,sEvent *ch3, sCurrentSequenceState *pSeqPtr);
void updateSequenceStep(void); 
void onEndSeq(void); //end sequence handler
void printHelpScreen(void);

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
  uint32 iError=amInit();
 
  if(iError!=1) return -1;
  
  printHelpScreen();
  turnOffKeyclick();

  //prepare sequence
  sEvent *ch1=getTestSequenceChannel(0);
  sEvent *ch2=getTestSequenceChannel(1);
  sEvent *ch3=getTestSequenceChannel(2);

  initSampleSequence(ch1,ch2,ch3,&g_CurrentState);

  IkbdClearState();
    
  // Install our asm ikbd handler 
  Supexec(IkbdInstall);
  
  //enter main loop
  while(bQuit!=TRUE)
  {
    
  for (uint16 i=0; i<IKBD_TABLE_SIZE; ++i) 
  {
     
     if (Ikbd_keyboard[i]==KEY_PRESSED) 
     {
	     Ikbd_keyboard[i]=KEY_UNDEFINED;
	
	switch(i){
	  case SC_ESC:
    {
	    bQuit=TRUE;
	  }break;
	  case SC_1:
    {
	    onToggleMidiEnable();
 	  }break;
	  case SC_2:
    {
	    onToggleYmEnable();
	  }break;
	  case SC_ARROW_UP:
    {
	    onTempoUp(&g_CurrentState);
	  }break;
	  case SC_ARROW_DOWN:
    {
	    onTempoDown(&g_CurrentState);
	  }break;
	  case SC_I:
    {
	    printHelpScreen();
	  }break;
	  case SC_M:
    {
	    onTogglePlayMode(&g_CurrentState);
	  }break;
	  case SC_P:
    {
	    onTogglePlayPauseSequence(&g_CurrentState);
	  }break;
	  case SC_SPACEBAR:
    {
	      onStopSequence(&g_CurrentState);
        sEvent *ch1=getTestSequenceChannel(0);
        sEvent *ch2=getTestSequenceChannel(1);
        sEvent *ch3=getTestSequenceChannel(2);

        initSampleSequence(ch1,ch2,ch3,&g_CurrentState);
	  }break;
	  
	} //end switch
     
     } // end if
     
    } //end for
    
    
  }//end while

  amAllNotesOff(16);
  Supexec(flushMidiSendBuffer);
  
  ymSoundOff();
  deinstallReplayRoutGeneric();

  /* Uninstall our asm handler */
  Supexec(IkbdUninstall);

  /* clean up, free internal library buffers etc..*/
  amDeinit();
   
 return 0;
}

void onTogglePlayMode(sCurrentSequenceState *pState)
{
  // toggle play mode PLAY ONCE / LOOP
  if(pState->state==S_PLAY_LOOP){
      amPrintf("Play sequence once."NL);
      pState->state=S_PLAY_ONCE;
    }
    else if(g_CurrentState.state==S_PLAY_ONCE){
      amPrintf("Play sequence in loop."NL);
      pState->state=S_PLAY_LOOP;
   }
   else
   {
	   if(pState->playMode==S_PLAY_LOOP)
     {
	     amPrintf("Play sequence once."NL);
	     pState->playMode=S_PLAY_ONCE;
	   }
	   else if(g_CurrentState.playMode==S_PLAY_ONCE)
     {
	     amPrintf("Play sequence in loop."NL);
	     pState->playMode=S_PLAY_LOOP;
	 }
  }
}

void onTempoUp(sCurrentSequenceState *pSeqPtr)
{
  uint32 iCurrentStep=0L;
  uint32 iCurrentTempo=0L;

  if(handleTempoChange==TRUE) return;

  iCurrentTempo=pSeqPtr->currentTempo;

  if(pSeqPtr->state==PS_STOPPED) return;
  
  if(iCurrentTempo<=0UL) 
  {
    pSeqPtr->currentTempo=0L;
    return;
  }
    
  if((iCurrentTempo<=50000UL&&iCurrentTempo>5000UL))
  {
      iCurrentStep=5000UL;
  }
  else if(iCurrentTempo<=5000UL)
  {
      iCurrentStep=100UL;
  }
  else
  { 
      iCurrentStep=TEMPO_STEP;
  }
  
  if(!((iCurrentTempo-iCurrentStep)<=0UL))
  {
      iCurrentTempo=iCurrentTempo-iCurrentStep;
      pSeqPtr->currentTempo=iCurrentTempo;
  }

  amPrintf("qn duration: %u [ms]"NL,iCurrentTempo);
  handleTempoChange=TRUE;    
}

void onTempoDown(sCurrentSequenceState *pSeqPtr)
{
uint32 iCurrentStep=0L;
uint32 iCurrentTempo=0L;

if(handleTempoChange==TRUE) return;

if(g_CurrentState.state==PS_STOPPED) return;

iCurrentTempo=pSeqPtr->currentTempo;

  if(iCurrentTempo<=50000UL)
  {
    iCurrentStep=5000UL;
  }else if(iCurrentTempo>50000UL)
  {
    iCurrentStep=TEMPO_STEP;  
  } 
  
  iCurrentTempo=iCurrentTempo+iCurrentStep;
  pSeqPtr->currentTempo=iCurrentTempo;
  
  amPrintf("qn duration: %u [ms]"NL,iCurrentTempo);

  handleTempoChange=TRUE;
}

void onToggleMidiEnable(void)
{
  amPrintf("MIDI output ");
  if(midiOutputEnabled==TRUE){
    midiOutputEnabled=FALSE;
    amAllNotesOff(16);
    amPrintf("disabled."NL);
   }else{
    midiOutputEnabled=TRUE;
    amPrintf("enabled."NL);
   }
}

void onToggleYmEnable(void)
{
  amPrintf("ym2149 output ");
  
  if(ymOutputEnabled==TRUE)
  {
    ymOutputEnabled=FALSE;
    ymSoundOff();
    amPrintf("disabled."NL);
  }
  else
  {
    ymOutputEnabled=TRUE;
    amPrintf("enabled."NL);
  }
}

void onTogglePlayPauseSequence(sCurrentSequenceState *pSeqPtr)
{

  amPrintf("Pause/Resume sequence"NL);
  
  if(pSeqPtr->state==PS_STOPPED)
  {
      pSeqPtr->state=PS_PLAYING;
  }
  else if(pSeqPtr->state==PS_PLAYING)
  {
      pSeqPtr->state=PS_PAUSED;
  }
  else if(pSeqPtr->state==PS_PAUSED)
  {
      pSeqPtr->state=PS_PLAYING;
  }
}

void onStopSequence(sCurrentSequenceState *pSeqPtr)
{
  amPrintf("Stop sequence"NL);
  
  pSeqPtr->state=PS_STOPPED;
  pSeqPtr->currentBPM=DEFAULT_BPM;
  pSeqPtr->currentTempo=DEFAULT_MPQN;
  pSeqPtr->currentPPQN=DEFAULT_PPQN;
  pSeqPtr->timeElapsedFrac=0UL;

  pSeqPtr->timeStep=amCalculateTimeStep(g_CurrentState.currentBPM, g_CurrentState.currentPPQN, SEQUENCER_UPDATE_HZ);
  
  for (uint16 i=0;i<3;++i){
      pSeqPtr->tracks[i].seqPosIdx=0UL;
      pSeqPtr->tracks[i].timeElapsedInt=0UL;
  }  
  
  amAllNotesOff(16);
  ymSoundOff();
}

bool isEndSeq(sEvent *pEvent)
{
  if((pEvent->delta==0&&pEvent->note==0))
    return TRUE;
  else 
    return FALSE;
}

void AM_INLINE printHelpScreen(void)
{
  amPrintf("==============================================="NL);
  amPrintf("/|\\ delta timing and sound output test.."NL);

#if AMIDILIB_USE_LIBC
    amPrintf("build date: %s %s"NL,__DATE__,__TIME__);
#else  
    amPrintf("build date: %s %s nolibc"NL,__DATE__,__TIME__);
#endif
 
  amPrintf("[arrow up/ arrow down] - change tempo " NL "\t500 ms/PQN and 96PPQN"NL);
  amPrintf("[1/2] - enable/disable midi out/ym2149 output "NL);
  amPrintf("[m] - toggle [PLAY ONCE/LOOP] sequence replay mode "NL);
  amPrintf("[p] - pause/resume sequence "NL);
  amPrintf("[i] - show this help screen "NL);
  
  amPrintf("[spacebar] - turn off all sounds / stop sequence "NL);
  amPrintf("[Esc] - quit"NL);
  amPrintf("(c) Nokturnal 2007-22"NL);
  amPrintf("================================================"NL);
}

// plays sample sequence 
int initSampleSequence(sEvent *ch1,sEvent *ch2,sEvent *ch3, sCurrentSequenceState *pSeqPtr)
{
  static bool bPlayModeInit=FALSE;
  uint8 mode=0,data=0;
  handleTempoChange=FALSE;
  funcPtrVoidVoid replayRout=customSeqReplay;
 
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
  pSeqPtr->timeStep=amCalculateTimeStep((uint16)DEFAULT_BPM, (uint16)DEFAULT_PPQN, (uint16)SEQUENCER_UPDATE_HZ);
  
   if(bPlayModeInit!=TRUE)
   {
     //init but only once, user can switch this option during runtime
      bPlayModeInit=TRUE;
      g_CurrentState.playMode=S_PLAY_LOOP; 
    }
  
  getMFPTimerSettings(SEQUENCER_UPDATE_HZ,&mode,&data);

  //install replay routine 
  installReplayRoutGeneric(mode, data, replayRout);

  return 0;
}



void updateSequenceStep(void)
{
static bool endOfSequence=FALSE;
static bool bStopped=FALSE;
  
  //check sequence state if paused do nothing
  if(g_CurrentState.state==PS_PAUSED) 
  {
    if(midiOutputEnabled==TRUE) amAllNotesOff(16);
    if(ymOutputEnabled==TRUE) ymSoundOff();
    return;
  }
  
  if(g_CurrentState.state==PS_PLAYING) bStopped=FALSE;
  //check sequence state if stopped reset position on all tracks
  //and reset tempo to default, but only once
  
  if((g_CurrentState.state==PS_STOPPED&&bStopped!=TRUE))
  {
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
    
    g_CurrentState.timeStep=amCalculateTimeStep(DEFAULT_BPM, DEFAULT_PPQN, SEQUENCER_UPDATE_HZ);
    
    if(midiOutputEnabled==TRUE) amAllNotesOff(16);
    if(ymOutputEnabled==TRUE) ymSoundOff();
    
    return;
  }
  else if(g_CurrentState.state==PS_STOPPED&&bStopped==TRUE)
  {
    return;
  }
  
  if(handleTempoChange==TRUE)
  {
    g_CurrentState.currentBPM=60000000/g_CurrentState.currentTempo;
    g_CurrentState.timeStep=amCalculateTimeStep(g_CurrentState.currentBPM, g_CurrentState.currentPPQN, SEQUENCER_UPDATE_HZ);
    //amTrace(NL "Set new timestep:%d"NL,g_CurrentState.timeStep);
    handleTempoChange=FALSE;
  }
  
   g_CurrentState.timeElapsedFrac += g_CurrentState.timeStep;
   uint32 TimeAdd = g_CurrentState.timeElapsedFrac >> 16;
   g_CurrentState.timeElapsedFrac &= 0xffff;
   
  //repeat for each track
  for (uint16 i=0;i<3;++i)
  {
     //for each active track
      uint32 count=g_CurrentState.tracks[i].seqPosIdx;
      sEvent *pEvent=&(g_CurrentState.tracks[i].seqPtr[count]);
      
      g_CurrentState.tracks[i].timeElapsedInt+=TimeAdd;
      
      while( ((isEndSeq(pEvent)!=TRUE)&&pEvent->delta<=g_CurrentState.tracks[i].timeElapsedInt))
      {
        endOfSequence=FALSE;
        g_CurrentState.tracks[i].timeElapsedInt -= pEvent->delta;
	  
        if(g_CurrentState.tracks[i].state.bIsActive==TRUE)
        {
            playNote(i+1,pEvent->note,midiOutputEnabled,ymOutputEnabled);
        }

        ++count;
        pEvent=&(g_CurrentState.tracks[i].seqPtr[count]);
      }
      
      //check for end of sequence
      if(isEndSeq(pEvent))
      {
        endOfSequence=TRUE;
        playNote(i+1,0,midiOutputEnabled,ymOutputEnabled);
      }
      else
      {
        g_CurrentState.tracks[i].seqPosIdx=count;
      }
  }
  
  //check if we have end of sequence
  //on all tracks
  if(endOfSequence==TRUE)
  {
    onEndSeq();
    endOfSequence=FALSE;
  }
}

void onEndSeq(void)
{

  if(g_CurrentState.playMode==S_PLAY_ONCE)
  {
      //reset set state to stopped 
      //reset song position on all tracks
      g_CurrentState.state=PS_STOPPED;
      onStopSequence(&g_CurrentState);
      return;
  }
  else if(g_CurrentState.playMode==S_PLAY_LOOP)
  {
      g_CurrentState.state=PS_PLAYING;  
      
      g_CurrentState.currentPPQN=DEFAULT_PPQN;
      g_CurrentState.currentBPM=60000000/g_CurrentState.currentTempo;  //do not reset current tempo !!!!
      g_CurrentState.timeElapsedFrac=0UL;
      g_CurrentState.timeStep=amCalculateTimeStep(g_CurrentState.currentBPM, DEFAULT_PPQN, SEQUENCER_UPDATE_HZ); 
  
      for (uint16 i=0;i<3;++i)
      {
        g_CurrentState.tracks[i].seqPosIdx=0UL;
        g_CurrentState.tracks[i].timeElapsedInt=0UL;
      }
       
      if(midiOutputEnabled==TRUE) amAllNotesOff(16);
      if(ymOutputEnabled==TRUE) ymSoundOff();
   }
}
