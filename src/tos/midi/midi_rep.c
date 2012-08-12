
//tos version
#include <lzo/lzoconf.h>

#include "timing/mfp.h"

#include "amidilib.h"
#include "midi_send.h"
#include "config.h"
#include "timing/miditim.h"

#include <math.h>
extern void midiSeqReplay(void);

extern volatile BOOL midiOutEnabled;
extern volatile BOOL ymOutEnabled;

extern U8 MIDIsendBuffer[]; //buffer from which we will send all data from the events once per frame
extern U16 MIDIbytesToSend; 
extern U16 MIDIbufferReady; //flag indicating buffer ready for sending data

volatile BOOL handleTempoChange;
static BOOL bTempoChanged=FALSE;
sSequence_t *g_CurrentSequence;

void initSeq(sSequence_t **seq){
 
 if(*seq!=0){
    U8 activeTrack=(*seq)->ubActiveTrack;
    U8 mode=0,data=0;
    g_CurrentSequence=0;
    g_CurrentSequence=(*seq);

    for(int i=0;i<(*seq)->ubNumTracks;i++){
     (*seq)->arTracks[i]->currentState.currentPPQN=DEFAULT_PPQN;
     (*seq)->arTracks[i]->currentState.currentTempo=DEFAULT_MPQN;
     (*seq)->arTracks[i]->currentState.currentBPM=DEFAULT_BPM;
     (*seq)->arTracks[i]->currentState.currentSeqPos=0;

     (*seq)->arTracks[i]->currentState.playState=PS_STOPPED;
     (*seq)->arTracks[i]->currentState.playMode=S_PLAY_LOOP;
    } 
  
    (*seq)->timeElapsedFrac=0;
    (*seq)->timeStep=am_calculateTimeStep((U16)DEFAULT_BPM, (U16)DEFAULT_PPQN, (U16)SEQUENCER_UPDATE_HZ);
  
#ifndef PORTABLE
    getMFPTimerSettings(SEQUENCER_UPDATE_HZ,&mode,&data);

    #ifdef DEBUG_BUILD
    amTrace("%dhz update interval, Time step: %d\r\n",SEQUENCER_UPDATE_HZ,(*seq)->timeStep);
    amTrace("calculated mode: %d, data: %d\n",mode,data);
    #endif  
    
    //install replay routine 
    installReplayRout(mode, data, midiSeqReplay);
#endif    
  }
  
 return;
}

BOOL isEOT(sEventList *pPtr){
  
  if(pPtr->eventBlock.type==T_META_EOT) return TRUE;
  
  return FALSE;
}

void onEndSequence(){
U8 activeTrack=0;
sTrackState_t *pTrackState;
activeTrack=g_CurrentSequence->ubActiveTrack;

pTrackState=&(g_CurrentSequence->arTracks[activeTrack]->currentState);

  if(pTrackState->playMode==S_PLAY_ONCE){
      //reset set state to stopped
      //reset song position on all tracks
      pTrackState->playState=PS_STOPPED;
      
    }else if(pTrackState->playMode==S_PLAY_LOOP){
      pTrackState->playState=PS_PLAYING;
    }
    
    if(midiOutEnabled==TRUE) am_allNotesOff(16);
    
    //reset all tracks state
    g_CurrentSequence->timeElapsedFrac=0;
    g_CurrentSequence->timeStep=am_calculateTimeStep(DEFAULT_BPM, DEFAULT_PPQN, SEQUENCER_UPDATE_HZ); 
     
    for (int i=0;i<g_CurrentSequence->ubNumTracks;i++){
      g_CurrentSequence->arTracks[i]->currentState.currentSeqPos=0;
      g_CurrentSequence->arTracks[i]->currentState.currentPPQN=DEFAULT_PPQN;
      g_CurrentSequence->arTracks[i]->currentState.currentTempo=DEFAULT_MPQN;
      g_CurrentSequence->arTracks[i]->currentState.currentBPM=DEFAULT_BPM;
    }  
}

void updateStep(){
 BOOL endOfSequence=FALSE;
 static BOOL bStopped=FALSE;
 U8 activeTrack=g_CurrentSequence->ubActiveTrack;

 sTrackState_t *pActiveTrackState=&(g_CurrentSequence->arTracks[activeTrack]->currentState);
  
 //check sequence state if paused do nothing
  if(pActiveTrackState->playState==PS_PAUSED) {
    if(midiOutEnabled==TRUE) am_allNotesOff(16);
    return;
  }
  
  if(pActiveTrackState->playState==PS_PLAYING) 
  bStopped=FALSE;
  //check sequence state if stopped reset position on all tracks
  //and reset tempo to default, but only once
  
  if(pActiveTrackState->playState==PS_STOPPED&&bStopped==FALSE){
    bStopped=TRUE;
    //repeat for each track
    for (int i=0;i<g_CurrentSequence->ubNumTracks;i++){
     
      g_CurrentSequence->arTracks[i]->currentState.currentPPQN=DEFAULT_PPQN;
      g_CurrentSequence->arTracks[i]->currentState.currentTempo=DEFAULT_MPQN;
      g_CurrentSequence->arTracks[i]->currentState.currentBPM=DEFAULT_BPM;
      g_CurrentSequence->arTracks[i]->currentState.currentSeqPos=0;
    }

    g_CurrentSequence->timeElapsedFrac=0;
    
    //reset tempo to initial valueas taken during start(get them from main sequence?)
    g_CurrentSequence->timeStep=am_calculateTimeStep((U16)DEFAULT_BPM, (U16)DEFAULT_PPQN, (U16)SEQUENCER_UPDATE_HZ);
    
    return;
  }
  else if(pActiveTrackState->playState==PS_STOPPED){
    return;
  }
  
  if(handleTempoChange==TRUE){
    pActiveTrackState->currentBPM=60000000/pActiveTrackState->currentTempo;
    g_CurrentSequence->timeStep=am_calculateTimeStep(pActiveTrackState->currentBPM, pActiveTrackState->currentPPQN, SEQUENCER_UPDATE_HZ);
    handleTempoChange=FALSE;
  }
  
   g_CurrentSequence->timeElapsedFrac += g_CurrentSequence->timeStep;
   U32 TimeAdd = g_CurrentSequence->timeElapsedFrac >> 16;
   g_CurrentSequence->timeElapsedFrac &= 0xffff;
   
  //repeat for each track
  for (int i=0;i<g_CurrentSequence->ubNumTracks;i++){
    //for each active track
      U32 count=g_CurrentSequence->arTracks[i]->currentState.currentSeqPos;
      sEventList *pEvent=&(g_CurrentSequence->arTracks[i]->pTrkEventList[count]);
      sTrack_t *pTrack=g_CurrentSequence->arTracks[i];
      
      pTrack->currentState.timeElapsedInt+=TimeAdd;
      
      while((isEOT(pEvent)==FALSE)&&pEvent->eventBlock.uiDeltaTime <= pTrack->currentState.timeElapsedInt){
	  endOfSequence=FALSE;
	  pTrack->currentState.timeElapsedInt -= pEvent->eventBlock.uiDeltaTime;
	  
	  if(pTrack->currentState.bMute==FALSE){
	    //play note
	     evntFuncPtr myFunc=NULL;
#ifdef IKBD_MIDI_SEND_DIRECT
	     //execute callback which copies data to midi buffer (_MIDIsendBuffer)
	     myFunc= pEvent->eventBlock.copyEventCb.func;
	    (*myFunc)((void *)pEvent->eventBlock.dataPtr);
#else
	    //execute callback which sends data to midi out
	     myFunc= pEvent->eventBlock.sendEventCb.func;
	    (*myFunc)((void *)pEvent->eventBlock.dataPtr);
#endif	    
	  }else{
	    //silence whole channel
	    U8 ch = getChannelNbFromEventBlock (&pEvent->eventBlock);
	    if(ch!=127)  all_notes_off(ch);
	  }

	  ++count;
	  pEvent=&(g_CurrentSequence->arTracks[i]->pTrkEventList[count]);
      }
      
      //check for end of sequence
      if(isEOT(pEvent)){
	endOfSequence=TRUE;
      }
  
      g_CurrentSequence->arTracks[i]->currentState.currentSeqPos=count;
  }
  
  //check if we have end of sequence
  //on all tracks
  if(endOfSequence==TRUE){
    onEndSequence();
    endOfSequence=FALSE;
  }
}




//this will be called from an interrupt in each delta increment
// void sequenceUpdate(){
//  static sTrack_t *pTrk=0;
//  static sEventList *pCurrent=0;
//  static evntFuncPtr myFunc=0; 
//  static sSequenceState_t *seqState=0;
//  static const U8 MIDI_START=0xFA;
//  static const U8 MIDI_STOP=0xFC;
//  
//  BOOL bNoteOffSent=FALSE;		//flag for sending note off events only once when pausing or stopping the sequence
//  static U32 silenceThrCounter=0;	//for tracking silence lenght on all tracks
//  
//  if(pCurrentSequence){ 
//   
//   U8 activeTrack=pCurrentSequence->ubActiveTrack;
//   seqState=&(pCurrentSequence->arTracks[activeTrack]->currentState);
//   
//   switch(seqState->playState){
//     
//     case PS_PLAYING:{
//     
//     bNoteOffSent=FALSE; //for handling PS_STOPPED and PS_PAUSED states
//     
//     //for each track (0-> (numTracks-1) ) 
//     for (U32 i=0;i<pCurrentSequence->ubNumTracks;i++){
//       pTrk=pCurrentSequence->arTracks[i];
// 
//       if(pTrk!=0){
// 	pCurrent=pTrk->currentState.pCurrent;
// 		
// 	//sometimes eventlist will be empty, because there will be nothing interesting
// 	//to send in parsed file, usually in the first track of MIDI 1 file (for example) 
// 	if(pCurrent!=NULL){ 
// 	
// 	//if (internal counter == current event delta)
// 	if((pCurrent->eventBlock.uiDeltaTime)==(pTrk->currentState.deltaCounter)){
// #ifdef DEBUG_BUILD 
// 	printEventBlock(&(pCurrent->eventBlock));
// #endif		
// 	if(!(pTrk->currentState.bMute)){
// 	  //output data only when track is not mute
// 	  myFunc= pCurrent->eventBlock.copyEventCb.func;
// 	  (*myFunc)(pCurrent->eventBlock.dataPtr);
// 	}else{
// 	  //silence whole channel
// 	  U8 ch = getChannelNbFromEventBlock (&pCurrent->eventBlock);
// 	  if(ch!=127)  all_notes_off(ch);
// 	}
// 		    
// 	pCurrent=pCurrent->pNext;
// 	pTrk->currentState.pCurrent=pCurrent;
// 	
// 	//check if next event isn't NULL, if yes do nothing
// 	// else check if event delta==0 if yes keep sending events, otherwise there is nothing to do
// 	while(((pCurrent!=0)&&(pCurrent->eventBlock.uiDeltaTime==0))){
// 	  //send data with delta==0
// #ifdef DEBUG_BUILD 
// 	printEventBlock(&(pCurrent->eventBlock));
// #endif		
// 	  if(!(pTrk->currentState.bMute)){
// 	    //the same as above
// 	    myFunc= pCurrent->eventBlock.copyEventCb.func;
// 	    (*myFunc)(pCurrent->eventBlock.dataPtr);
// 	  }else{
// 	    //silence whole channel
// 	    U8 ch = getChannelNbFromEventBlock (&pCurrent->eventBlock);
// 	    if(ch!=127)  all_notes_off(ch);
// 	  }
// 	  
// 	  //next
// 	  pCurrent=pCurrent->pNext;
// 	  pTrk->currentState.pCurrent=pCurrent;
// 	}
// 	// done reset internal track counter
// 	// pMidiTune->arTracks[i]->currentState.pCurrent should point to event with NULL or
// 	// event with delta higher than 0
// 	pTrk->currentState.deltaCounter=0;
// #ifdef DEBUG_BUILD 
// 	amTrace((const U8 *)"reset track: %d counter\n",i);
// #endif
// 	}else{
// 	// else internal counter++; 
// 	++pCurrentSequence->arTracks[i]->currentState.deltaCounter;
// #ifdef DEBUG_BUILD 
// 	amTrace((const U8 *)"increase track %d counter\n",i);
// #endif
// 	}
// 	}
// #ifdef DEBUG_BUILD 
// else{
// 	//quite normal
// 	amTrace((const U8 *)"Nothing to send in this track..\n");}
// #endif
// }
// }//track iteration
// 
//     if(MIDIbytesToSend!=0){
// 	MIDI_SEND_DATA(MIDIbytesToSend-1,MIDIsendBuffer);
// 	MIDIbytesToSend=0;
//     }
//     
//       //handle tempo/time signature update
//     //are there any time signature or tempo updates?
//     //if yes recalculate tempo
//     if(bTimeSignatureChanged||bTempoChanged){
//       U8 mode=0,data=0;
//        
//       calculateTempo(seqState,&mode, &data);
//       tbMode=mode;
//       tbData=data;
//       
//       //reset flags
//       bTimeSignatureChanged=FALSE;
//       bTempoChanged=FALSE;
//     }
//       //increase our cumulated delta
//       ++pCurrentSequence->accumulatedDeltaCounter;
//       
//     }break;
//     case PS_PAUSED:{
//       
//          if(bNoteOffSent==FALSE){
// 	  //turn all notes off on external module but only once
// 	  bNoteOffSent=TRUE;
// 	  am_allNotesOff(16);
// 	}
//       
//     }break;
//     case PS_STOPPED:{
//       silenceThrCounter=0;
//       
//       //reset all counters, but only once
//       if(bNoteOffSent==FALSE){
// 	  pCurrentSequence->accumulatedDeltaCounter=0;
// 	  
// 	  for (U32 i=0;i<pCurrentSequence->ubNumTracks;i++){
// 	    pTrk=pCurrentSequence->arTracks[i];
// 	    pTrk->currentState.deltaCounter=0;
// 	    pTrk->currentState.pCurrent=pTrk->currentState.pStart;
// 	     
// 	    //reset default midi tempo settings
// 	    //and recalculate the tempo
// 	    setDefaultMidiTempoSettings( &pTrk->currentState);
// 	    
// 	    U8 mode=0,data=0;
//        
// 	    calculateTempo(seqState,&mode, &data);
// 	    tbMode=mode;
// 	    tbData=data;
// 
// 	  }
// 	  //turn all notes off on external module
// 	  bNoteOffSent=TRUE;
// 	  startPlaying=1;	//to indicate that we have to send MIDI start on next play
// 	  MIDI_SEND_BYTE(MIDI_STOP);	//send midi STOP
// 	  am_allNotesOff(16);
// 	//done! 
// 	}
//     }break;
//   };
//  }//pCurrentSequence null check
// }


// this will be called from an interrupt in each delta increment
// replay version with sending data directly to device

// void sequenceUpdate2(void){
//   
//  static sTrack_t *pTrk=0;
//  static sEventList *pCurrent=0;
//  static evntFuncPtr myFunc=0; 
//  static sSequenceState_t *seqState=0;
//  static const U8 MIDI_START=0xFA;
//  static const U8 MIDI_STOP=0xFC;
//  
//  BOOL bNoteOffSent=FALSE;		//flag for sending note off events only once when pausing or stopping the sequence
//  static U32 silenceThrCounter=0;	//for tracking silence lenght on all tracks
//  
//  if(pCurrentSequence){ 
//   
//   U8 activeTrack=pCurrentSequence->ubActiveTrack;
//   seqState=&(pCurrentSequence->arTracks[activeTrack]->currentState);
//   
//   switch(seqState->playState){
//     
//     case PS_PLAYING:{
//     
//     bNoteOffSent=FALSE; //for handling PS_STOPPED and PS_PAUSED states
//     
//     //for each track (0-> (numTracks-1) ) 
//     for (U32 i=0;i<pCurrentSequence->ubNumTracks;i++){
//       pTrk=pCurrentSequence->arTracks[i];
// 
//       if(pTrk!=0){
// 	pCurrent=pTrk->currentState.pCurrent;
// 		
// 	//sometimes eventlist will be empty, because there will be nothing interesting
// 	//to send in parsed file, usually in the first track of MIDI 1 file (for example) 
// 	if(pCurrent!=NULL){ 
// 	
// 	//if (internal counter == current event delta)
// 	if((pCurrent->eventBlock.uiDeltaTime)==(pTrk->currentState.deltaCounter)){
// #ifdef DEBUG_BUILD 
// 	printEventBlock(&(pCurrent->eventBlock));
// #endif		
// 	if(!(pTrk->currentState.bMute)){
// 	  //output data only when track is not mute
// 	  myFunc= pCurrent->eventBlock.sendEventCb.func;
// 	  (*myFunc)((void *)pCurrent->eventBlock.dataPtr);
// 	}else{
// 	     //silence whole channel
// 	    U8 ch = getChannelNbFromEventBlock (&pCurrent->eventBlock);
// 	    if(ch!=127)  all_notes_off(ch);
// 	}
// 		    
// 	pCurrent=pCurrent->pNext;
// 	pTrk->currentState.pCurrent=pCurrent;
// 	
// 	//check if next event isn't NULL, if yes do nothing
// 	// else check if event delta==0 if yes keep sending events, otherwise there is nothing to do
// 	while(((pCurrent!=0)&&(pCurrent->eventBlock.uiDeltaTime==0))){
// 	  //send data with delta==0
// #ifdef DEBUG_BUILD 
// 	printEventBlock(&(pCurrent->eventBlock));
// #endif		
// 	  if(!(pTrk->currentState.bMute)){
// 	    //the same as above
// 	    myFunc= pCurrent->eventBlock.sendEventCb.func;
// 	    (*myFunc)((void *)pCurrent->eventBlock.dataPtr);
// 	  }else{
// 	    //silence whole channel
// 	    U8 ch = getChannelNbFromEventBlock (&pCurrent->eventBlock);
// 	    if(ch!=127)  all_notes_off(ch);
// 	  }
// 	//next
// 	  pCurrent=pCurrent->pNext;
// 	  pTrk->currentState.pCurrent=pCurrent;
// 	}
// 	// done reset internal track counter
// 	// pMidiTune->arTracks[i]->currentState.pCurrent should point to event with NULL or
// 	// event with delta higher than 0
// 	pTrk->currentState.deltaCounter=0;
// #ifdef DEBUG_BUILD 
// 	amTrace((const U8 *)"reset track: %d counter\n",i);
// #endif
// 	}else{
// 	// else internal counter++; 
// 	++pCurrentSequence->arTracks[i]->currentState.deltaCounter;
// #ifdef DEBUG_BUILD 
// 	amTrace((const U8 *)"increase track %d counter\n",i);
// #endif
// 	}
// 	}
// #ifdef DEBUG_BUILD 
// else{
// 	//quite normal
// 	amTrace((const U8 *)"Nothing to send in this track..\n");}
// #endif
// }
// }//track iteration
//        //handle tempo/time signature update
//     //are there any time signature or tempo updates?
//     //if yes recalculate tempo
//     if(bTimeSignatureChanged||bTempoChanged){
//       U8 mode=0,data=0;
//        
//       calculateTempo(seqState,&mode, &data);
//       tbMode=mode;
//       tbData=data;
//       
//       //reset flags
//       bTimeSignatureChanged=FALSE;
//       bTempoChanged=FALSE;
//     }
//     
//       //increase our cumulated delta
//       ++pCurrentSequence->accumulatedDeltaCounter;
//       
//     }break;
//     case PS_PAUSED:{
//       
//          if(bNoteOffSent==FALSE){
// 	  //turn all notes off on external module but only once
// 	  bNoteOffSent=TRUE;
// 	  am_allNotesOff(16);
// 	}
//       
//     }break;
//     case PS_STOPPED:{
//       silenceThrCounter=0;
//       
//       //reset all counters, but only once
//       if(bNoteOffSent==FALSE){
// 	  pCurrentSequence->accumulatedDeltaCounter=0;
// 	  
// 	  for (U32 i=0;i<pCurrentSequence->ubNumTracks;i++){
// 	    pTrk=pCurrentSequence->arTracks[i];
// 	    pTrk->currentState.deltaCounter=0;
// 	    pTrk->currentState.pCurrent=pTrk->currentState.pStart;
// 	    
// 	    //reset default midi tempo settings
// 	    //and recalculate the tempo
// 	    setDefaultMidiTempoSettings( &pTrk->currentState);
// 	    
// 	    U8 mode=0,data=0;
//        
// 	    calculateTempo(seqState,&mode, &data);
// 	    tbMode=mode;
// 	    tbData=data;
// 	  }
// 	  //turn all notes off on external module
// 	  bNoteOffSent=TRUE;
// 	  startPlaying=1;	//to indicate that we have to send MIDI start on next play
// 	  MIDI_SEND_BYTE(MIDI_STOP);	//send midi STOP
// 	  am_allNotesOff(16);
// 	//done! 
// 	}
//     }break;
//   };
//  }//pCurrentSequence null check
// }//sequenceUpdate2() end

//replay control
BOOL isSeqPlaying(void){
  if(g_CurrentSequence!=0){
    U8 activeTrack=g_CurrentSequence->ubActiveTrack;
    if((g_CurrentSequence->arTracks[activeTrack]->currentState.playState==PS_PLAYING)) 
      return TRUE;
    else 
      return FALSE;
  }
  return FALSE;
}


void stopSeq(void){
  if(g_CurrentSequence!=0){
    U8 activeTrack=g_CurrentSequence->ubActiveTrack;
    if(g_CurrentSequence->arTracks[activeTrack]->currentState.playState!=PS_STOPPED){
      g_CurrentSequence->arTracks[activeTrack]->currentState.playState=PS_STOPPED;
    }
  }
}

void pauseSeq(){
  sTrack_t *pTrack=0;
  //printf("Pause/Resume.\n");
  if(g_CurrentSequence!=0){
    // TODO: handling individual tracks for MIDI 2 type
    // for one sequence(single/multichannel) we will check state of the first track only
    U8 activeTrack=g_CurrentSequence->ubActiveTrack;
    pTrack=g_CurrentSequence->arTracks[activeTrack];
    
    switch(pTrack->currentState.playState){
      case PS_PLAYING:{
	pTrack->currentState.playState=PS_PAUSED;
      }break;
      case PS_PAUSED:{
	pTrack->currentState.playState=PS_PLAYING;
      }break;
    };
  }
}

void playSeq(void){
  if(g_CurrentSequence!=0){
    //set state
    U8 activeTrack=g_CurrentSequence->ubActiveTrack;
    if(g_CurrentSequence->arTracks[activeTrack]->currentState.playState==PS_STOPPED)
      g_CurrentSequence->arTracks[activeTrack]->currentState.playState=PS_PLAYING;
  }
}

void muteTrack(U16 trackNb,BOOL bMute){
  if(((g_CurrentSequence!=0)&&(trackNb<AMIDI_MAX_TRACKS))){
    g_CurrentSequence->arTracks[trackNb]->currentState.bMute=bMute;
  }
}

void toggleReplayMode(void){
  
  if(g_CurrentSequence!=0){
    U8 activeTrack=g_CurrentSequence->ubActiveTrack;
 
    switch(g_CurrentSequence->arTracks[activeTrack]->currentState.playMode){
      case S_PLAY_ONCE: {
	g_CurrentSequence->arTracks[activeTrack]->currentState.playMode=S_PLAY_LOOP;
      }break;
      case S_PLAY_LOOP: {
	g_CurrentSequence->arTracks[activeTrack]->currentState.playMode=S_PLAY_ONCE;
      }break;
    }
  }
}

void getCurrentSeq (sSequence_t **pPtr){
  (*pPtr)=g_CurrentSequence;
}

