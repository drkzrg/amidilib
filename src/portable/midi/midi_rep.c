

#include <math.h>

//portable version 
#include "midi_rep.h"

#include "amidilib.h"
#include "amidiseq.h"
#include "midi_send.h"
#include "list/list.h"


sSequence_t *pCurrentSequence;	//here is stored current sequence
U8 MIDIsendBuffer[]; //buffer from which we will send all data from the events
		     //once per 
U16 MIDIbufferReady; //flag indicating buffer ready for sending data
U16 startPlaying;

void update(){
//TODO! 
return;
}


void initSeq(sSequence_t *seq){
  //TODO! 
 return;
}



//this will be called from an interrupt in each delta increment
void sequenceUpdate(){
#warning TODO! ;>
}

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


void installReplayRout(U8 mode,U8 data,void *fn){
#warning installReplayRout() not implemented!
}

void deinstallReplayRout(void){
#warning deinstallReplayRout() not implemented!
}


