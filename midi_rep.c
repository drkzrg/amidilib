
#include "include/midi_rep.h"
#include "mfp.h"
#include "amidiseq.h"

static volatile U32 deltaCounter;
extern volatile sSequence_t *pCurrentSequence;

#ifndef PORTABLE
extern volatile U8 _tbData;
extern volatile U8 _tbMode;
#endif

void initSeq(sSequence_t *seq){
#ifdef PORTABLE
  //TODO! 
#else
if(seq!=0){
  U8 activeTrack=seq->ubActiveTrack;
  U32 freq=seq->arTracks[activeTrack]->currentState.currentTempo/seq->timeDivision;
  U32 mode=0,data=0;
  
  pCurrentSequence=seq;
  
  getMFPTimerSettings(freq/8,&mode,&data);
  installReplayRout(mode, data);
}
#endif
 return;
}


//this will be called from an interrupt in each delta increment
void sequenceUpdate(void){
 static sTrack_t *pTrk=0;
 static sEventList *pCurrent=0;
 static evntFuncPtr myFunc=0; 
 BOOL bNoteOffSent=FALSE;		//flag for sending note off events only once when pausing or stopping the sequence
 static U32 silenceThrCounter=0;	//for tracking silence lenght on all tracks
 
 if(pCurrentSequence){ 
   //TODO: change it to assert and include only in DEBUG build
  U8 activeTrack=pCurrentSequence->ubActiveTrack;
  switch(pCurrentSequence->arTracks[activeTrack]->currentState.playState){
    
    case PS_PLAYING:{
	bNoteOffSent=FALSE; //for handling PS_STOPPED and PS_PAUSED states
    
    //for each track (0-> (numTracks-1) ) 
    for (U32 i=0;i<pCurrentSequence->ubNumTracks;i++){
      pTrk=pCurrentSequence->arTracks[i];

      if(pTrk!=0){
	//TODO: adjust handling for multiple, independent tracks
	//TODO: [optional] slap all the pointers to an array and send everything in one go
	//TODO: rewrite this in m68k :P for speeeeed =) so cooooool....
	
	pCurrent=pTrk->currentState.pCurrent;
		
	//sometimes eventlist will be empty, because there will be nothing interesting
	//to send in parsed file, usually in the first track of MIDI 1 file (for example) 
	if(pCurrent!=NULL){ 
	
	//if (internal counter == current event delta)
	if((pCurrent->eventBlock.uiDeltaTime)==pTrk->currentState.deltaCounter){
#ifdef DEBUG_BUILD 
	printEventBlock(&(pCurrent->eventBlock));
#endif		
	if(pTrk->currentState.bMute!=TRUE){
	  //output data only when track is not mute
	  myFunc= pCurrent->eventBlock.infoBlock.func;
	  (*myFunc)((void *)pCurrent->eventBlock.dataPtr);
	}else{
	 ;  //TODO: silence whole channel
	 //all_notes_off(U8 channel)
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
	  if(pTrk->currentState.bMute!=TRUE){
	    //the same as above
	    myFunc= pCurrent->eventBlock.infoBlock.func;
	    (*myFunc)((void *)pCurrent->eventBlock.dataPtr);
	  }else{
	   ; //TODO: silence whole channel
	   //all_notes_off(U8 channel)
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
	pCurrentSequence->arTracks[i]->currentState.deltaCounter++;
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
    sSequenceState_t *pState=&(pCurrentSequence->arTracks[activeTrack]->currentState);
    
    if(pState->currentTempo!=pState->newTempo){
      //update track current tempo
      pState->currentTempo=pState->newTempo;
      
      //update timer data feed
      //TODO:
      
      
    }
    
    /////////////////////// check for end of the track 
    if(silenceThrCounter==pCurrentSequence->eotThreshold){
      //we have meet the threshold, time to decide what to do next
      // if we play in loop mode: do not change actual state,reset track to the beginning
      
      switch(pCurrentSequence->arTracks[activeTrack]->currentState.playMode){
	
	case S_PLAY_LOOP:{
	 
	  for (U32 i=0;i<pCurrentSequence->ubNumTracks;i++){
	    pTrk=pCurrentSequence->arTracks[i];
	    pTrk->currentState.deltaCounter=0;
	    pTrk->currentState.pCurrent=pTrk->currentState.pStart;
	  }
	  // notes off we could maybe do other things too
	am_allNotesOff(16);
	silenceThrCounter=0;
	}break;
	case S_PLAY_ONCE:{
	    //set state only, the rest will be done in next interrupt
	  pCurrentSequence->arTracks[activeTrack]->currentState.playState=PS_STOPPED;
	  silenceThrCounter=0;
	}break;
      };
    }else{ 
      //silence threshold not met, check if all the tracks events are null if yes increase silenceThrCounter
      //else reset it
      BOOL bNothing=FALSE;
      for (U32 i=0;i<pCurrentSequence->ubNumTracks;i++){
	pTrk=pCurrentSequence->arTracks[i];
	//TODO: incorporate it in the playing state loop, so we can get rid of track iteration (?)
	if(pTrk!=0){
	  if(pTrk->currentState.pCurrent!=0){
	    bNothing=FALSE;
	  }else 
	    bNothing=TRUE;
      }
      /////////////////////////// end of eot check
     
    }//end of track loop
     if(bNothing==TRUE){
	    silenceThrCounter++;
	  }else 
	    silenceThrCounter=0;
    }//end of silence threshold not met
	deltaCounter++;
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
      deltaCounter=0;
      //reset all counters, but only once
      if(bNoteOffSent==FALSE){
	  for (U32 i=0;i<pCurrentSequence->ubNumTracks;i++){
	    pTrk=pCurrentSequence->arTracks[i];
	    pTrk->currentState.deltaCounter=0;
	    pTrk->currentState.pCurrent=pTrk->currentState.pStart;
	  }
	  //turn all notes off on external module
	  bNoteOffSent=TRUE;
	  am_allNotesOff(16);
	//done! 
	}
    }break;
  };
 }//pCurrentSequence null check
}//sequenceUpdate() end

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
    //TODO: handling individual tracks for MIDI 2 type
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

#ifdef PORTABLE
void installReplayRout(U8 mode,U8 data){
//todo  
#warning installReplayRout() unimplemented! 
}


void deinstallReplayRout(void){
//TODO!
#warning deinstallReplayRout() unimplemented! 
}

#endif
