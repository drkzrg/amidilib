
// MIDI replay core

/**  Copyright 2007-2020 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#include "config.h"
#include "amidilib.h"
#include "timing/miditim.h"
#include "timing/mfp.h"
#include "midi_send.h"
#include "containers/list.h"

#include <stdio.h>

static sSequence_t *g_CurrentSequence=0;

void getCurrentSeq(sSequence_t **pSeq){
  *pSeq=g_CurrentSequence;
}

void initSeq(sSequence_t *seq, const eTimerType timerType){
 g_CurrentSequence=0;

if(seq!=0){
    uint8 mode=0,data=0;
    sTrack_t *pTrack=0;
    sTrackState_t *pTrackState=0;
    uint8 activeTrack=seq->ubActiveTrack;

    g_CurrentSequence=seq;

    Supexec(clearMidiOutputBuffer);

    for(int i=0;i<seq->ubNumTracks;++i){
        pTrack=seq->arTracks[i];

        if(pTrack){
            pTrackState=&(pTrack->currentState);
            pTrackState->currentTempo=DEFAULT_MPQN;
            pTrackState->currentBPM=DEFAULT_BPM;
            pTrackState->timeElapsedInt=0L;
            pTrackState->currEventPtr=pTrack->pTrkEventList; //set begining of event list
            pTrackState->playState = getGlobalConfig()->initialTrackState&(~(TM_MUTE));

        }
    } 
  
    seq->timeElapsedFrac=0L;
    seq->timeStep=0L;
    pTrackState=&(seq->arTracks[activeTrack]->currentState);
    seq->timeStep=amCalculateTimeStep(pTrackState->currentBPM, seq->timeDivision, SEQUENCER_UPDATE_HZ);

#ifdef IKBD_MIDI_SEND_DIRECT
        Supexec(flushMidiSendBuffer);
#endif

    getMFPTimerSettings(SEQUENCER_UPDATE_HZ,&mode,&data);

#ifdef DEBUG_BUILD
    amTrace("%dhz update interval, Time step: %d\r\n",SEQUENCER_UPDATE_HZ,seq->timeStep);
    amTrace("calculated mode: %d, data: %d\n",mode,data);
#endif
    
    if(seq->seqType==ST_SINGLE){
        installReplayRout(mode, data, FALSE, timerType);
    }else if(seq->seqType==ST_MULTI){
        installReplayRout(mode, data, TRUE, timerType);
    }else{
         amTrace("initSeq(): Houston, we have a problem!\n");
        // Houston, we have a problem!
        assert(0);
    }

  } //endif
 return;
}

void initSeqManual(sSequence_t *seq){

 if(seq!=0){
	sTrackState_t *pTrackState=0;
    //uint8 activeTrack=seq->ubActiveTrack;

    g_CurrentSequence=0;
    g_CurrentSequence=seq;

    for(uint16 i=0;i<seq->ubNumTracks;++i){
	 sTrack_t *pTrack=0;
     pTrack=seq->arTracks[i];

        if(pTrack){
            pTrackState=&(pTrack->currentState);
            pTrackState->currentTempo=DEFAULT_MPQN;
            pTrackState->currentBPM=DEFAULT_BPM;
            pTrackState->timeElapsedInt=0L;
            pTrackState->currEventPtr=pTrack->pTrkEventList;               // set begining of event list
            pTrackState->playState = getGlobalConfig()->initialTrackState&(~(TM_MUTE));

        }

    } 
  
    seq->timeElapsedFrac=0L;
    seq->timeStep=0L;
    seq->timeStep = amCalculateTimeStep(seq->arTracks[0]->currentState.currentBPM, seq->timeDivision, SEQUENCER_UPDATE_HZ);

#ifdef IKBD_MIDI_SEND_DIRECT
        Supexec(flushMidiSendBuffer);
#endif

  }
  
 return;
}

bool isEOT(volatile const sEventList *pPtr){
  if(pPtr->eventBlock.type==T_META_EOT) return TRUE;

  return FALSE;
}

void onEndSequence(void){

sTrackState_t *pTrackState=0;
sTrack_t *pTrack=0;

if(g_CurrentSequence){
	uint8 activeTrack=0;
    activeTrack=g_CurrentSequence->ubActiveTrack;
    pTrackState=&(g_CurrentSequence->arTracks[activeTrack]->currentState);

      if(pTrackState->playState&TM_PLAY_ONCE){
          //reset set state to stopped
          //reset song position on all tracks
          pTrackState->playState&=(~(TS_PS_PLAYING|TS_PS_PAUSED));
        }else { // loop
          pTrackState->playState&=(~TS_PS_PAUSED);
          pTrackState->playState|=TS_PS_PLAYING;
        }

        // reset all controllers

        amAllNotesOff(16);

        for (uint16 i=0;i<g_CurrentSequence->ubNumTracks;++i){

          pTrack=g_CurrentSequence->arTracks[i];

          if(pTrack){
              pTrack->currentState.timeElapsedInt=0L;
              pTrack->currentState.currentTempo=DEFAULT_MPQN;
              pTrack->currentState.currentBPM=DEFAULT_BPM;
              pTrackState->currEventPtr=pTrack->pTrkEventList; //set begining of event list
          }
        }

#ifdef IKBD_MIDI_SEND_DIRECT
        flushMidiSendBuffer();
#endif
        // reset all tracks state
        g_CurrentSequence->timeElapsedFrac=0L;
        g_CurrentSequence->timeStep=amCalculateTimeStep(pTrackState->currentBPM, g_CurrentSequence->timeDivision, SEQUENCER_UPDATE_HZ);
    }

}

volatile static bool bEventSent=FALSE;
volatile static bool bSend=FALSE;
volatile static bool bEOTflag=FALSE;
volatile static bool bStopped=FALSE;
volatile static bool endOfSequence=FALSE;
volatile static uint32 TimeAdd=0;
volatile static evntFuncPtr myFunc=NULL;
volatile static uint32 currentDelta=0;
volatile static sEventList *pCurrentEvent=0;
volatile static uint32 timeElapsed=0;
volatile static sTrackState_t *pActiveTrackState=0;
volatile static sTrack_t *pTrack=0;

// single track handler
void updateStepSingle(void){
 bStopped=FALSE;

 if(g_CurrentSequence==0) return;

 //get track, there is only one active
 pTrack=g_CurrentSequence->arTracks[0];
 pActiveTrackState=&(pTrack->currentState);

 //check sequence state if paused do nothing
  if(pActiveTrackState->playState&TS_PS_PAUSED) {
    amAllNotesOff(16);

#ifdef IKBD_MIDI_SEND_DIRECT
  flushMidiSendBuffer();
#endif

    return;
  }

  if(pActiveTrackState->playState&TS_PS_PLAYING){
     bStopped=FALSE;
  }

  if(!(pActiveTrackState->playState&TS_PS_PLAYING)){
     //check sequence state if stopped reset position on all tracks
     //and reset tempo to default, but only once

     if(bStopped==FALSE){
          bStopped=TRUE;
          pActiveTrackState=0;
          pTrack=0;

          //reset track
          pTrack=g_CurrentSequence->arTracks[0];

          if(pTrack){
                pActiveTrackState=&(pTrack->currentState);
                pActiveTrackState->currentTempo=DEFAULT_MPQN;
                pActiveTrackState->currentBPM=DEFAULT_BPM;
                pActiveTrackState->timeElapsedInt=0L;
           }

          g_CurrentSequence->timeElapsedFrac=0L;
          g_CurrentSequence->timeStep=0L;

          //reset tempo to initial valueas taken during start(get them from main sequence?)
          g_CurrentSequence->timeStep=amCalculateTimeStep(pActiveTrackState->currentBPM,g_CurrentSequence->timeDivision, SEQUENCER_UPDATE_HZ);

          //rewind to the first event
          while(pActiveTrackState->currEventPtr->pPrev!=0){
              pActiveTrackState->currEventPtr=pActiveTrackState->currEventPtr->pPrev;
          }
     }
    return;
  }

  bStopped=FALSE; //we replaying, so we have to reset this flag

   pCurrentEvent=pActiveTrackState->currEventPtr;

   bEOTflag=isEOT(pActiveTrackState->currEventPtr);
   timeElapsed=pActiveTrackState->timeElapsedInt;
//reset
   myFunc=NULL;
   bEventSent=FALSE;
   bSend=FALSE;
   currentDelta=pCurrentEvent->eventBlock.uiDeltaTime;

   if(currentDelta==timeElapsed) bSend=TRUE;


if(bEOTflag==FALSE&&bSend!=FALSE){
    endOfSequence=FALSE;

#ifdef IKBD_MIDI_SEND_DIRECT
    //execute callback which copies data to midi buffer (_MIDIsendBuffer)
    myFunc=pCurrentEvent->eventBlock.copyEventCb.func;
    //printEventBlock(&pCurrentEvent->eventBlock);
    (*myFunc)((void *)pCurrentEvent->eventBlock.dataPtr);
#else
    //execute callback which sends data directly to midi out (XBIOS)
    myFunc= pCurrentEvent->eventBlock.sendEventCb.func;
    //printEventBlock(&pActiveTrackState->currEventPtr->eventBlock);
    (*myFunc)((void *)pCurrentEvent->eventBlock.dataPtr);
#endif

   //go to next event
   pActiveTrackState->currEventPtr=pActiveTrackState->currEventPtr->pNext;
   pCurrentEvent=pActiveTrackState->currEventPtr;

   //check end of track
   if(pCurrentEvent!=0){

      bEOTflag=isEOT(pCurrentEvent);

    //check if next events are null and pack buffer until first next non zero delta
    while( pCurrentEvent && ((bEOTflag!=FALSE) && (pCurrentEvent->eventBlock.uiDeltaTime==0)) ){
        //handle event
#ifdef IKBD_MIDI_SEND_DIRECT
    //execute callback which copies data to midi buffer (_MIDIsendBuffer)
    myFunc=pCurrentEvent->eventBlock.copyEventCb.func;
    //printEventBlock(&pActiveTrackState->currEventPtr->eventBlock);
    (*myFunc)((void *)pCurrentEvent->eventBlock.dataPtr);
#else
    //execute callback which sends data directly to midi out (XBIOS)
    myFunc= pCurrentEvent->eventBlock.sendEventCb.func;
    //printEventBlock(&pActiveTrackState->currEventPtr->eventBlock);
    (*myFunc)((void *)pCurrentEvent->eventBlock.dataPtr);
#endif

        //go to next event
        pActiveTrackState->currEventPtr=pActiveTrackState->currEventPtr->pNext;
        pCurrentEvent=pActiveTrackState->currEventPtr;

        if(pCurrentEvent) bEOTflag=isEOT(pCurrentEvent);
    }

    if(bEOTflag!=FALSE){
      endOfSequence=TRUE;
    }

    bEventSent=TRUE;
   }
  } //endif

    g_CurrentSequence->timeElapsedFrac += g_CurrentSequence->timeStep;
    TimeAdd = g_CurrentSequence->timeElapsedFrac >> 16;
    g_CurrentSequence->timeElapsedFrac &= 0xffff;

    if(TimeAdd>1)TimeAdd=1;


   //add time elapsed
   if(bEventSent!=FALSE){
     pActiveTrackState->timeElapsedInt=0;
   }else{
     pActiveTrackState->timeElapsedInt=pActiveTrackState->timeElapsedInt+TimeAdd;
   }

  //check if we have end of sequence
  //on all tracks
  if(endOfSequence!=FALSE){
    onEndSequence();
    endOfSequence=FALSE;
    amTrace("End of Sequence\n");
  }

} //end UpdateStep()

// multitrack handler
void updateStepMulti(void){
    bStopped=FALSE;

    if(g_CurrentSequence==0) return;

    uint8 numOfTracks=g_CurrentSequence->ubNumTracks;

// get status from first track
    pTrack=g_CurrentSequence->arTracks[0];
    pActiveTrackState=&(pTrack->currentState);

    //check sequence state if paused do nothing
     if(pActiveTrackState->playState&TS_PS_PAUSED) {
       amAllNotesOff(16);

#ifdef IKBD_MIDI_SEND_DIRECT
  flushMidiSendBuffer();
#endif
       return;
     }

     if(pActiveTrackState->playState&TS_PS_PLAYING){
        bStopped=FALSE;
     }

     if(!(pActiveTrackState->playState&TS_PS_PLAYING)){
        //check sequence state if stopped reset position on all tracks
        //and reset tempo to default, but only once

        if(bStopped==FALSE){
             bStopped=TRUE;
             pActiveTrackState=0;
             pTrack=0;

             //reset track
             pTrack=g_CurrentSequence->arTracks[0];

             if(pTrack){
                   pActiveTrackState=&(pTrack->currentState);
                   pActiveTrackState->currentTempo=DEFAULT_MPQN;
                   pActiveTrackState->currentBPM=DEFAULT_BPM;
                   pActiveTrackState->timeElapsedInt=0L;
              }

             g_CurrentSequence->timeElapsedFrac=0L;
             g_CurrentSequence->timeStep=0L;

             //reset tempo to initial valueas taken during start(get them from main sequence?)
             g_CurrentSequence->timeStep=amCalculateTimeStep(pActiveTrackState->currentBPM,g_CurrentSequence->timeDivision, SEQUENCER_UPDATE_HZ);

             //rewind to the first event
             while(pActiveTrackState->currEventPtr->pPrev!=0){
                 pActiveTrackState->currEventPtr=pActiveTrackState->currEventPtr->pPrev;
             }
        }
       return;
     }

     bStopped=FALSE; //we replaying, so we have to reset this flag

     // repeat for each track
     for(int iTrackNb=0;iTrackNb<numOfTracks;++iTrackNb){

        pTrack=g_CurrentSequence->arTracks[iTrackNb];
        pActiveTrackState=&(pTrack->currentState);

        if(pTrack){

            pCurrentEvent=pActiveTrackState->currEventPtr;

            if(pCurrentEvent){
                bEOTflag=isEOT(pActiveTrackState->currEventPtr);
                timeElapsed=pActiveTrackState->timeElapsedInt;

                //reset
                myFunc=NULL;
                bEventSent=FALSE;
                bSend=FALSE;

                currentDelta=pCurrentEvent->eventBlock.uiDeltaTime;
                if(currentDelta==timeElapsed) bSend=TRUE;

                if(bEOTflag==FALSE&&bSend!=FALSE){
                endOfSequence=FALSE;

   #ifdef IKBD_MIDI_SEND_DIRECT
            //execute callback which copies data to midi buffer (_MIDIsendBuffer)
            myFunc=pCurrentEvent->eventBlock.copyEventCb.func;
            //printEventBlock(&pCurrentEvent->eventBlock);
            (*myFunc)((void *)pCurrentEvent->eventBlock.dataPtr);
   #else
            //execute callback which sends data directly to midi out (XBIOS)
            myFunc= pCurrentEvent->eventBlock.sendEventCb.func;
            //printEventBlock(&pActiveTrackState->currEventPtr->eventBlock);
            (*myFunc)((void *)pCurrentEvent->eventBlock.dataPtr);
   #endif

      //go to next event
            pActiveTrackState->currEventPtr=pActiveTrackState->currEventPtr->pNext;
            pCurrentEvent=pActiveTrackState->currEventPtr;

      //check end of track
      if(pCurrentEvent!=0){
                bEOTflag=isEOT(pCurrentEvent);

                //check if next events are null and pack buffer until first next non zero delta
                while( (pCurrentEvent!=0) && ((bEOTflag!=FALSE) && (pCurrentEvent->eventBlock.uiDeltaTime==0)) ){

                //handle event
#ifdef IKBD_MIDI_SEND_DIRECT
                // execute callback which copies data to midi buffer (_MIDIsendBuffer)
                    myFunc = pCurrentEvent->eventBlock.copyEventCb.func;
                    (*myFunc)((void *)pCurrentEvent->eventBlock.dataPtr);
#else
                // execute callback which sends data directly to midi out (XBIOS)
                    myFunc = pCurrentEvent->eventBlock.sendEventCb.func;
                    (*myFunc)((void *)pCurrentEvent->eventBlock.dataPtr);
#endif
                //go to next event
                    pActiveTrackState->currEventPtr = pActiveTrackState->currEventPtr->pNext;
                    pCurrentEvent = pActiveTrackState->currEventPtr;

                    if(pCurrentEvent) bEOTflag = isEOT(pCurrentEvent);

                } //end while

                if(bEOTflag!=FALSE){
                    endOfSequence=TRUE;
                }

                bEventSent=TRUE;
        }
      } //endif

      g_CurrentSequence->timeElapsedFrac += g_CurrentSequence->timeStep;
      TimeAdd = g_CurrentSequence->timeElapsedFrac >> 16;
      g_CurrentSequence->timeElapsedFrac &= 0xffff;

      if(TimeAdd>1)TimeAdd=1;

      //add time elapsed
        if(bEventSent!=FALSE){
            pActiveTrackState->timeElapsedInt=0;
        }else{
            pActiveTrackState->timeElapsedInt=pActiveTrackState->timeElapsedInt+TimeAdd;
        }
       } // cur event null check
      } //end pTrack null check
     } // repeat for each track >> end

     //check if we have end of sequence
     //on all tracks
     if(endOfSequence!=FALSE){
       onEndSequence();
       endOfSequence=FALSE;
       amTrace("End of Sequence\n");
     }

}

//replay control
bool isSeqPlaying(void){
  if(g_CurrentSequence!=0){
	sTrack_t *pTrack=0;
    uint8 activeTrack=g_CurrentSequence->ubActiveTrack;
    pTrack=g_CurrentSequence->arTracks[activeTrack];

    if(pTrack){
        if(pTrack->currentState.playState&TS_PS_PLAYING)
          return TRUE;
        else
          return FALSE;
    }
  }
  return FALSE;
}


void stopSeq(void){
  sTrack_t *pTrack=0;

  if(g_CurrentSequence!=0){
    uint8 activeTrack=g_CurrentSequence->ubActiveTrack;
    pTrack=g_CurrentSequence->arTracks[activeTrack];

    if(pTrack){
        const sTrackState_t *tState=&(pTrack->currentState);
        if( ((tState->playState&TS_PS_PLAYING)||(tState->playState&TS_PS_PAUSED)) ){
          pTrack->currentState.playState&=(~(TS_PS_PLAYING|TS_PS_PAUSED));
          printf("Stop sequence\n");
        }
    }
  }

  //all notes off
  amAllNotesOff(16);

#ifdef IKBD_MIDI_SEND_DIRECT
  Supexec(flushMidiSendBuffer);
#endif

}

void pauseSeq(void){
  //printf("Pause/Resume.\n");
  if(g_CurrentSequence!=0){
    uint8 activeTrack=0;
	sTrack_t *pTrack=0;
    // TODO: handling individual tracks for MIDI 2 type
    // for one sequence( single / multichannel) we will check state of the first track only
    activeTrack=g_CurrentSequence->ubActiveTrack;
    pTrack=g_CurrentSequence->arTracks[activeTrack];

    if(pTrack){
         uint16 state=pTrack->currentState.playState;

         if((state&TS_PS_PLAYING)&&(!(state&TS_PS_PAUSED)))
         {
          pTrack->currentState.playState&=(uint16)(~TS_PS_PLAYING);
          pTrack->currentState.playState|=(uint16)TS_PS_PAUSED;

          printf("Pause sequence\n");
          return;
         }else if( !(state&TS_PS_PLAYING)&&(state&TS_PS_PAUSED) ){
           pTrack->currentState.playState&=(uint16)(~TS_PS_PAUSED); //unpause
           pTrack->currentState.playState|=(uint16)TS_PS_PLAYING;  //set playing state
           printf("Resume sequence\n");
         }
    }
  }

}//pauseSeq

void playSeq(void){
 if(g_CurrentSequence!=0){
    //set state
    uint8 activeTrack=g_CurrentSequence->ubActiveTrack;
    sTrack_t *pTrack=g_CurrentSequence->arTracks[activeTrack];

    if(pTrack){
        uint16 state=pTrack->currentState.playState;

        if(!(state&TS_PS_PLAYING)) {

            pTrack->currentState.playState&=(~(TS_PS_PAUSED));
            pTrack->currentState.playState|=TS_PS_PLAYING;

            printf("Play sequence\t");

            if(pTrack->currentState.playState&TM_PLAY_ONCE){
              printf("[ ONCE ]\n");
            }else{
              printf("[ LOOP ]\n");
            }
        }
    }

  }
}

void muteTrack(const uint16 trackNb, const bool bMute){
  if(((g_CurrentSequence!=0)&&(trackNb<AMIDI_MAX_TRACKS))){
    if(bMute!=FALSE){
        g_CurrentSequence->arTracks[trackNb]->currentState.playState|=TM_MUTE;
        printf("Mute track %d\n",trackNb);
    }else{
        g_CurrentSequence->arTracks[trackNb]->currentState.playState&=(~TM_MUTE);
        printf("UnMute track %d\n",trackNb);
    }
  }
}

void toggleReplayMode(void){

  if(g_CurrentSequence!=0){
    uint8 activeTrack=0;
	sTrack_t *pTrack=0;
	
    activeTrack=g_CurrentSequence->ubActiveTrack;
    pTrack=g_CurrentSequence->arTracks[activeTrack];
    uint16 state=pTrack->currentState.playState;

    if(state&TM_PLAY_ONCE){
       pTrack->currentState.playState&=(~TM_PLAY_ONCE);
       printf("Set replay mode: [ LOOP ]\n");
    }else{
        pTrack->currentState.playState|=TM_PLAY_ONCE;
        printf("Set replay mode: [ ONCE ]\n");
    }
  }
}

void amDestroySequence (sSequence_t **pPtr){
  #ifdef DEBUG_BUILD
    amTrace((const uint8 *)"amDestroySequence() destroy sequence at %p initiated... 1..2..3... \n",*pPtr);
  #endif

  //go to the end of sequence
  if((*pPtr)->pSequenceName!=0){
      amFree(((*pPtr)->pSequenceName));
  }

  //destroy all tracks
   for (uint16 i=0;i<AMIDI_MAX_TRACKS;++i){
     if((*pPtr)->arTracks[i]!=0){
     if((*pPtr)->arTracks[i]->pTrackName!=0) amFree(((*pPtr)->arTracks[i]->pTrackName));

#ifdef EVENT_LINEAR_BUFFER
      destroyList((*pPtr),&((*pPtr)->arTracks[i]->pTrkEventList));
#else
      destroyList(&((*pPtr)->arTracks[i]->pTrkEventList));
#endif

      amFree(((*pPtr)->arTracks[i]));
     }
   }

  //destroy sequence and nullify it
  amFree(*pPtr);
  #ifdef DEBUG_BUILD
    amTrace((const uint8 *)"amDestroySequence() done. \n");
  #endif
}

void printSequenceState(void){

if(g_CurrentSequence){

    printf("Td/PPQN: %u\n",g_CurrentSequence->timeDivision);
    printf("Time step: %lu\n",g_CurrentSequence->timeStep);
    printf("Time elapsedFrac: %lu\n",g_CurrentSequence->timeElapsedFrac);

    sTrack_t *pTrack=0;

    switch(g_CurrentSequence->seqType){

    case ST_SINGLE:{
      pTrack=g_CurrentSequence->arTracks[0];

      if(pTrack){
		 sTrackState_t *pTrackState=0;
         printf("Track state:\n");
		 
         pTrackState=&(pTrack->currentState);
         printf("\tTime elapsed: %lu\n",pTrackState->timeElapsedInt);
         printf("\tCur BPM: %lu\n",pTrackState->currentBPM);
         printf("\tCur Tempo: %lu\n",pTrackState->currentTempo);
         printf("\tCur Play state: %s\n",getPlayStateStr(pTrackState->playState));
         printf("\tMute: %s\n",(pTrackState->playState & TM_MUTE)?"TRUE":"FALSE");
      }
    }break;
    case ST_MULTI:{
		sTrackState_t *pTrackState=0;
        pTrack=g_CurrentSequence->arTracks[g_CurrentSequence->ubActiveTrack];
        pTrackState=&(pTrack->currentState);

        printf("Nb of tracks: %d\n",g_CurrentSequence->ubNumTracks);
        printf("Active track: %d\n",g_CurrentSequence->ubActiveTrack);
        printf("Cur Play state: %s\n",getPlayStateStr(pTrackState->playState));
        printf("Cur Tempo: %lu\n",pTrackState->currentTempo);
        printf("Cur BPM: %lu\n",pTrackState->currentBPM);

        for (uint16 i=0;i<g_CurrentSequence->ubNumTracks;++i){
            pTrack=g_CurrentSequence->arTracks[i];
            pTrackState=0;

            if(pTrack){
                printf("Track[%d]\t",i);
                pTrackState=&(pTrack->currentState);
                printf("\tTime elapsed: %lu\t",pTrackState->timeElapsedInt);
                printf("\tMute: %s\n",pTrackState->playState&TM_MUTE?"TRUE":"FALSE");
            }
        }
    }break;
    case ST_MULTI_SUB:{
		
        printf("Nb of tracks: %d\n",g_CurrentSequence->ubNumTracks);
        printf("Active track: %d\n",g_CurrentSequence->ubActiveTrack);

        for (uint16 i=0;i<g_CurrentSequence->ubNumTracks;++i){
            pTrack=g_CurrentSequence->arTracks[i];
				sTrackState_t *pTrackState=0;
                printf("Track[%d]\n",i);
                pTrackState=&(pTrack->currentState);
                printf("Time elapsed: %lu\n",pTrackState->timeElapsedInt);
                printf("Cur BPM: %lu\n",pTrackState->currentBPM);
                printf("Cur Tempo: %lu\n",pTrackState->currentTempo);
                printf("Cur play state: %s\n",getPlayStateStr(pTrackState->playState));
                printf("\tMute: %s\n",pTrackState->playState&TM_MUTE?"TRUE":"FALSE");
            }
        } break;
   
	} // switch()
    
 
#ifdef DEBUG_BUILD
	printMidiSendBufferState();
#endif
}

}

const uint8 *getPlayStateStr(const uint16 state){

    if( !(state&TS_PS_PLAYING) && (state&TS_PS_PAUSED) ){
       return "Paused";
    }else if(state&TS_PS_PLAYING && (!(state&TS_PS_PAUSED))){
       return "Playing";
    }else if(!(state&TS_PS_PLAYING)){
       return "Stopped...";
    }
	
 return "";
}




