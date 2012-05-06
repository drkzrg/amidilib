
//tos version

#include "midi_rep.h"
#include "timing/mfp.h"

#include "amidilib.h"
#include "midi_send.h"
#include "config.h"

#include <math.h>

extern sSequence_t *pCurrentSequence;
extern volatile U16 startPlaying;
extern volatile U8 tbData;
extern volatile U8 tbMode;

void initSeq(sSequence_t *seq){

 if(seq!=0){
    U8 activeTrack=seq->ubActiveTrack;
    U8 mode=0,data=0;
    pCurrentSequence=seq;

    calculateTempo(&(seq->arTracks[activeTrack]->currentState),&mode,&data);
  
    pCurrentSequence->pulseCounter=0;  
    pCurrentSequence->divider=0;  //not used atm
  
    startPlaying=1;
    installReplayRout(mode, data);
  }
  
 return;
}

extern U8 MIDIsendBuffer[]; //buffer from which we will send all data from the events once per frame
extern U16 MIDIbytesToSend; 
extern U16 MIDIbufferReady; //flag indicating buffer ready for sending data

//this will be called from an interrupt in each delta increment
void sequenceUpdate(){
 static sTrack_t *pTrk=0;
 static sEventList *pCurrent=0;
 static evntFuncPtr myFunc=0; 
 static sSequenceState_t *seqState=0;
 static const U8 MIDI_START=0xFA;
 static const U8 MIDI_STOP=0xFC;
 
 BOOL bNoteOffSent=FALSE;		//flag for sending note off events only once when pausing or stopping the sequence
 static U32 silenceThrCounter=0;	//for tracking silence lenght on all tracks
 
 if(pCurrentSequence){ 
  
  U8 activeTrack=pCurrentSequence->ubActiveTrack;
  seqState=&(pCurrentSequence->arTracks[activeTrack]->currentState);
  
  switch(seqState->playState){
    
    case PS_PLAYING:{
    
    bNoteOffSent=FALSE; //for handling PS_STOPPED and PS_PAUSED states
    
    //for each track (0-> (numTracks-1) ) 
    for (U32 i=0;i<pCurrentSequence->ubNumTracks;i++){
      pTrk=pCurrentSequence->arTracks[i];

      if(pTrk!=0){
	pCurrent=pTrk->currentState.pCurrent;
		
	//sometimes eventlist will be empty, because there will be nothing interesting
	//to send in parsed file, usually in the first track of MIDI 1 file (for example) 
	if(pCurrent!=NULL){ 
	
	//if (internal counter == current event delta)
	if((pCurrent->eventBlock.uiDeltaTime)==(pTrk->currentState.deltaCounter)){
#ifdef DEBUG_BUILD 
	printEventBlock(&(pCurrent->eventBlock));
#endif		
	if(!(pTrk->currentState.bMute)){
	  //output data only when track is not mute
	  myFunc= pCurrent->eventBlock.copyEventCb.func;
	  (*myFunc)(pCurrent->eventBlock.dataPtr);
	}else{
	  //silence whole channel
	  U8 ch = getChannelNbFromEventBlock (&pCurrent->eventBlock);
	  if(ch!=127)  all_notes_off(ch);
	}
		    
	pCurrent=pCurrent->pNext;
	pTrk->currentState.pCurrent=pCurrent;
	
	//check if next event isn't NULL, if yes do nothing
	// else check if event delta==0 if yes keep sending events, otherwise there is nothing to do
	while(((pCurrent!=0)&&(pCurrent->eventBlock.uiDeltaTime==0))){
	  //send data with delta==0
#ifdef DEBUG_BUILD 
	printEventBlock(&(pCurrent->eventBlock));
#endif		
	  if(!(pTrk->currentState.bMute)){
	    //the same as above
	    myFunc= pCurrent->eventBlock.copyEventCb.func;
	    (*myFunc)(pCurrent->eventBlock.dataPtr);
	  }else{
	    //silence whole channel
	    U8 ch = getChannelNbFromEventBlock (&pCurrent->eventBlock);
	    if(ch!=127)  all_notes_off(ch);
	  }
	  
	  //next
	  pCurrent=pCurrent->pNext;
	  pTrk->currentState.pCurrent=pCurrent;
	}
	// done reset internal track counter
	// pMidiTune->arTracks[i]->currentState.pCurrent should point to event with NULL or
	// event with delta higher than 0
	pTrk->currentState.deltaCounter=0;
#ifdef DEBUG_BUILD 
	amTrace((const U8 *)"reset track: %d counter\n",i);
#endif
	}else{
	// else internal counter++; 
	++pCurrentSequence->arTracks[i]->currentState.deltaCounter;
#ifdef DEBUG_BUILD 
	amTrace((const U8 *)"increase track %d counter\n",i);
#endif
	}
	}
#ifdef DEBUG_BUILD 
else{
	//quite normal
	amTrace((const U8 *)"Nothing to send in this track..\n");}
#endif
}
}//track iteration

    if(MIDIbytesToSend!=0){
	MIDI_SEND_DATA(MIDIbytesToSend-1,MIDIsendBuffer);
	MIDIbytesToSend=0;
    }
    
    //handle tempo update
    if(seqState->currentTempo!=seqState->newTempo){
      U8 mode=0,data=0;
      
      //update track current tempo
      seqState->currentTempo=seqState->newTempo;
      
      calculateTempo(seqState,&mode, &data);
 
      tbMode=mode;
      tbData=data;
    }
      //increase our cumulated delta
      ++pCurrentSequence->accumulatedDeltaCounter;
      
    }break;
    case PS_PAUSED:{
      
         if(bNoteOffSent==FALSE){
	  //turn all notes off on external module but only once
	  bNoteOffSent=TRUE;
	  am_allNotesOff(16);
	}
      
    }break;
    case PS_STOPPED:{
      silenceThrCounter=0;
      
      //reset all counters, but only once
      if(bNoteOffSent==FALSE){
	  pCurrentSequence->accumulatedDeltaCounter=0;
	  
	  for (U32 i=0;i<pCurrentSequence->ubNumTracks;i++){
	    pTrk=pCurrentSequence->arTracks[i];
	    pTrk->currentState.deltaCounter=0;
	    pTrk->currentState.pCurrent=pTrk->currentState.pStart;
	  }
	  //turn all notes off on external module
	  bNoteOffSent=TRUE;
	  startPlaying=1;	//to indicate that we have to send MIDI start on next play
	  MIDI_SEND_BYTE(MIDI_STOP);	//send midi STOP
	  am_allNotesOff(16);
	//done! 
	}
    }break;
  };
 }//pCurrentSequence null check
}


// this will be called from an interrupt in each delta increment
// replay version with sending data directly to device

void sequenceUpdate2(void){
  
 static sTrack_t *pTrk=0;
 static sEventList *pCurrent=0;
 static evntFuncPtr myFunc=0; 
 static sSequenceState_t *seqState=0;
 static const U8 MIDI_START=0xFA;
 static const U8 MIDI_STOP=0xFC;
 
 BOOL bNoteOffSent=FALSE;		//flag for sending note off events only once when pausing or stopping the sequence
 static U32 silenceThrCounter=0;	//for tracking silence lenght on all tracks
 
 if(pCurrentSequence){ 
  
  U8 activeTrack=pCurrentSequence->ubActiveTrack;
  seqState=&(pCurrentSequence->arTracks[activeTrack]->currentState);
  
  switch(seqState->playState){
    
    case PS_PLAYING:{
    
    bNoteOffSent=FALSE; //for handling PS_STOPPED and PS_PAUSED states
    
    //for each track (0-> (numTracks-1) ) 
    for (U32 i=0;i<pCurrentSequence->ubNumTracks;i++){
      pTrk=pCurrentSequence->arTracks[i];

      if(pTrk!=0){
	pCurrent=pTrk->currentState.pCurrent;
		
	//sometimes eventlist will be empty, because there will be nothing interesting
	//to send in parsed file, usually in the first track of MIDI 1 file (for example) 
	if(pCurrent!=NULL){ 
	
	//if (internal counter == current event delta)
	if((pCurrent->eventBlock.uiDeltaTime)==(pTrk->currentState.deltaCounter)){
#ifdef DEBUG_BUILD 
	printEventBlock(&(pCurrent->eventBlock));
#endif		
	if(!(pTrk->currentState.bMute)){
	  //output data only when track is not mute
	  myFunc= pCurrent->eventBlock.sendEventCb.func;
	  (*myFunc)((void *)pCurrent->eventBlock.dataPtr);
	}else{
	     //silence whole channel
	    U8 ch = getChannelNbFromEventBlock (&pCurrent->eventBlock);
	    if(ch!=127)  all_notes_off(ch);
	}
		    
	pCurrent=pCurrent->pNext;
	pTrk->currentState.pCurrent=pCurrent;
	
	//check if next event isn't NULL, if yes do nothing
	// else check if event delta==0 if yes keep sending events, otherwise there is nothing to do
	while(((pCurrent!=0)&&(pCurrent->eventBlock.uiDeltaTime==0))){
	  //send data with delta==0
#ifdef DEBUG_BUILD 
	printEventBlock(&(pCurrent->eventBlock));
#endif		
	  if(!(pTrk->currentState.bMute)){
	    //the same as above
	    myFunc= pCurrent->eventBlock.sendEventCb.func;
	    (*myFunc)((void *)pCurrent->eventBlock.dataPtr);
	  }else{
	    //silence whole channel
	    U8 ch = getChannelNbFromEventBlock (&pCurrent->eventBlock);
	    if(ch!=127)  all_notes_off(ch);
	  }
	//next
	  pCurrent=pCurrent->pNext;
	  pTrk->currentState.pCurrent=pCurrent;
	}
	// done reset internal track counter
	// pMidiTune->arTracks[i]->currentState.pCurrent should point to event with NULL or
	// event with delta higher than 0
	pTrk->currentState.deltaCounter=0;
#ifdef DEBUG_BUILD 
	amTrace((const U8 *)"reset track: %d counter\n",i);
#endif
	}else{
	// else internal counter++; 
	++pCurrentSequence->arTracks[i]->currentState.deltaCounter;
#ifdef DEBUG_BUILD 
	amTrace((const U8 *)"increase track %d counter\n",i);
#endif
	}
	}
#ifdef DEBUG_BUILD 
else{
	//quite normal
	amTrace((const U8 *)"Nothing to send in this track..\n");}
#endif
}
}//track iteration
    //handle tempo update
    if(seqState->currentTempo!=seqState->newTempo){
      //update track current tempo
      seqState->currentTempo=seqState->newTempo;
      U8 mode=0,data=0;
     
      calculateTempo(seqState,&mode,&data);
      
      tbMode=mode;
      tbData=data;
    }
    
      //increase our cumulated delta
      ++pCurrentSequence->accumulatedDeltaCounter;
      
    }break;
    case PS_PAUSED:{
      
         if(bNoteOffSent==FALSE){
	  //turn all notes off on external module but only once
	  bNoteOffSent=TRUE;
	  am_allNotesOff(16);
	}
      
    }break;
    case PS_STOPPED:{
      silenceThrCounter=0;
      
      //reset all counters, but only once
      if(bNoteOffSent==FALSE){
	  pCurrentSequence->accumulatedDeltaCounter=0;
	  
	  for (U32 i=0;i<pCurrentSequence->ubNumTracks;i++){
	    pTrk=pCurrentSequence->arTracks[i];
	    pTrk->currentState.deltaCounter=0;
	    pTrk->currentState.pCurrent=pTrk->currentState.pStart;
	  }
	  //turn all notes off on external module
	  bNoteOffSent=TRUE;
	  startPlaying=1;	//to indicate that we have to send MIDI start on next play
	  MIDI_SEND_BYTE(MIDI_STOP);	//send midi STOP
	  am_allNotesOff(16);
	//done! 
	}
    }break;
  };
 }//pCurrentSequence null check
}//sequenceUpdate2() end

//replay control
BOOL isSeqPlaying(void){
  if(pCurrentSequence!=0){
    U8 activeTrack=pCurrentSequence->ubActiveTrack;
    if((pCurrentSequence->arTracks[activeTrack]->currentState.playState==PS_PLAYING)) 
      return TRUE;
    else 
      return FALSE;
  }
  return FALSE;
}


void stopSeq(void){
  if(pCurrentSequence!=0){
    U8 activeTrack=pCurrentSequence->ubActiveTrack;
    if(pCurrentSequence->arTracks[activeTrack]->currentState.playState!=PS_STOPPED){
      pCurrentSequence->arTracks[activeTrack]->currentState.playState=PS_STOPPED;
    }
  }
}

void pauseSeq(){
  sTrack_t *pTrack=0;
  //printf("Pause/Resume.\n");
  if(pCurrentSequence!=0){
    // TODO: handling individual tracks for MIDI 2 type
    // for one sequence(single/multichannel) we will check state of the first track only
    U8 activeTrack=pCurrentSequence->ubActiveTrack;
    pTrack=pCurrentSequence->arTracks[activeTrack];
    
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
  if(pCurrentSequence!=0){
    //set state
    U8 activeTrack=pCurrentSequence->ubActiveTrack;
    if(pCurrentSequence->arTracks[activeTrack]->currentState.playState==PS_STOPPED)
      pCurrentSequence->arTracks[activeTrack]->currentState.playState=PS_PLAYING;
  }
}

void muteTrack(U16 trackNb,BOOL bMute){
  if(((pCurrentSequence!=0)&&(trackNb<AMIDI_MAX_TRACKS))){
    pCurrentSequence->arTracks[trackNb]->currentState.bMute=bMute;
  }
}

void toggleReplayMode(void){
  
  if(pCurrentSequence!=0){
    U8 activeTrack=pCurrentSequence->ubActiveTrack;
 
    switch(pCurrentSequence->arTracks[activeTrack]->currentState.playMode){
      case S_PLAY_ONCE: {
	pCurrentSequence->arTracks[activeTrack]->currentState.playMode=S_PLAY_LOOP;
      }break;
      case S_PLAY_LOOP: {
	pCurrentSequence->arTracks[activeTrack]->currentState.playMode=S_PLAY_ONCE;
      }break;
    }
  }
}

void getCurrentSeq (sSequence_t **pPtr){
  (*pPtr)=pCurrentSequence;
}


void calculateTempo(const sSequenceState_t *pPtr,U8 *mode, U8 *data){
  
  float freq=pPtr->currentTempo/1000000.0f/pCurrentSequence->timeDivision;
  
  //calculate hz
  freq=1.0f/freq;	
 
  getMFPTimerSettings((U32)freq,mode,data);
 
  /*
   U32 dd,nn,cc;

  dd = pPtr->timeSignature.dd;
  nn = pPtr->timeSignature.nn;
  cc = pPtr->timeSignature.cc;
   
   * dd=(U32)pow(2.0f,(float)dd);
  //convert tempo from microseconds to seconds
  float freq=((float)(pPtr->currentTempo/(dd/4)*(cc/24))/1000000.0f);
  float fTempoSecs=pPtr->currentTempo/1000000.0f;
   
  //calculate one tick duration in seconds(quaternote duration)
  freq=(freq/(dd/4))/(pCurrentSequence->timeDivision/(dd/4))*nn;
      
  //calculate hz
  freq=1.0f/freq;	
  
  #ifdef DEBUG_BUILD
    amTrace("freq: %f %f\n",fTempoSecs,(U32)freq);
  #endif  
  
  getMFPTimerSettings((U32)fTempoSecs,mode,data);
  fprintf(stderr,"Set freq %f\tSet mode: %d data:%d\r\n",fTempoSecs,mode,data);
  */
  
  #ifdef DEBUG_BUILD
    amTrace("calculated mode: %d, data: %d\n",mode,data);
  #endif  
}
