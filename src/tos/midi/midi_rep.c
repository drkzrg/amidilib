
//tos version
#include <math.h>

#include "timing/mfp.h"
#include "amidilib.h"
#include "midi_send.h"
#include "config.h"
#include "timing/miditim.h"
#include "list/list.h"

extern void midiSeqReplay(void);

extern volatile BOOL midiOutEnabled;
extern volatile BOOL ymOutEnabled;

sSequence_t *g_CurrentSequence=0;

void getCurrentSeq(sSequence_t **pSeq){
  *pSeq=g_CurrentSequence;
}

#ifdef IKBD_MIDI_SEND_DIRECT
//clears custom midi output buffer
void clearMidiOutputBuffer(){
    MIDIbytesToSend=0;
    amMemSet(MIDIsendBuffer,0,MIDI_SENDBUFFER_SIZE*sizeof(U8));
}
#endif

void initSeq(sSequence_t *seq){
 g_CurrentSequence=0;

if(seq!=0){
    U8 mode=0,data=0;
    sTrack_t *pTrack=0;
    sTrackState_t *pTrackState=0;
    U8 activeTrack=seq->ubActiveTrack;

    g_CurrentSequence=seq;

    MIDIbytesToSend=0;

    for(int i=0;i<seq->ubNumTracks;++i){
        pTrack=seq->arTracks[i];

        if(pTrack){
            pTrackState=&(pTrack->currentState);
            pTrackState->currentTempo=DEFAULT_MPQN;
            pTrackState->currentBPM=DEFAULT_BPM;
            pTrackState->currentSeqPos=0L;
            pTrackState->timeElapsedInt=0L;
            pTrackState->bMute=FALSE;
            pTrackState->bTempoChanged=FALSE;
            pTrackState->currEventPtr=pTrack->pTrkEventList; //set begining of event list
            pTrackState->playState = getGlobalConfig()->playState;
            pTrackState->playMode = getGlobalConfig()->playMode;
        }
    } 
  
    seq->timeElapsedFrac=0L;
    seq->timeStep=0L;
    pTrackState=&(seq->arTracks[activeTrack]->currentState);
    seq->timeStep=am_calculateTimeStep(pTrackState->currentBPM, seq->timeDivision, SEQUENCER_UPDATE_HZ);

#ifdef IKBD_MIDI_SEND_DIRECT
        clearMidiOutputBuffer();
#endif

    getMFPTimerSettings(SEQUENCER_UPDATE_HZ,&mode,&data);

#ifdef DEBUG_BUILD
    amTrace("%dhz update interval, Time step: %d\r\n",SEQUENCER_UPDATE_HZ,seq->timeStep);
    amTrace("calculated mode: %d, data: %d\n",mode,data);
#endif
    
    //install replay routine 
    installReplayRout(mode, data, midiSeqReplay);
}
  
 return;
}


void initSeqManual(sSequence_t *seq){
sTrack_t *pTrack=0;
sTrackState_t *pTrackState=0;
U8 mode=0,data=0;

 if(seq!=0){
    U8 activeTrack=seq->ubActiveTrack;

    g_CurrentSequence=0;
    g_CurrentSequence=seq;

    for(int i=0;i<seq->ubNumTracks;++i){
     pTrack=seq->arTracks[i];

     if(pTrack){
         pTrackState=&(pTrack->currentState);
         pTrackState->currentTempo=DEFAULT_MPQN;
         pTrackState->currentBPM=DEFAULT_BPM;
         pTrackState->currentSeqPos=0L;
         pTrackState->timeElapsedInt=0L;
         pTrackState->bMute=FALSE;
         pTrackState->bTempoChanged=FALSE;
         pTrackState->currEventPtr=pTrack->pTrkEventList; //set begining of event list
         pTrackState->playState = getGlobalConfig()->playState;
         pTrackState->playMode = getGlobalConfig()->playMode;
     }
    } 
  
#ifdef IKBD_MIDI_SEND_DIRECT
        clearMidiOutputBuffer();
#endif

    seq->timeElapsedFrac=0L;
    seq->timeStep=0L;    
    seq->timeStep=am_calculateTimeStep(pTrackState->currentBPM, seq->timeDivision, SEQUENCER_UPDATE_HZ);
  }
  
 return;
}

BOOL isEOT(volatile const sEventList *pPtr){
  if(pPtr->eventBlock.type==T_META_EOT) return TRUE;

  return FALSE;
}

void onEndSequence(){
U8 activeTrack=0;
sTrackState_t *pTrackState=0;

sTrack_t *pTrack=0;

if(g_CurrentSequence){
    activeTrack=g_CurrentSequence->ubActiveTrack;
    pTrackState=&(g_CurrentSequence->arTracks[activeTrack]->currentState);

      if(pTrackState->playMode==S_PLAY_ONCE){
          //reset set state to stopped
          //reset song position on all tracks
          pTrackState->playState=PS_STOPPED;
        }else if(pTrackState->playMode==S_PLAY_LOOP){
          pTrackState->playState=PS_PLAYING;
        }

        am_allNotesOff(16);

        for (int i=0;i<g_CurrentSequence->ubNumTracks;++i){

          pTrack=g_CurrentSequence->arTracks[i];

          if(pTrack){
              pTrack->currentState.currentSeqPos=0L;
              pTrack->currentState.timeElapsedInt=0L;
              pTrack->currentState.currentTempo=DEFAULT_MPQN;
              pTrack->currentState.currentBPM=DEFAULT_BPM;
              pTrackState->bTempoChanged=FALSE;
              pTrackState->currEventPtr=pTrack->pTrkEventList; //set begining of event list
          }
        }

#ifdef IKBD_MIDI_SEND_DIRECT
        clearMidiOutputBuffer();
#endif
        // reset all tracks state
        g_CurrentSequence->timeElapsedFrac=0L;
        g_CurrentSequence->timeStep=am_calculateTimeStep(pTrackState->currentBPM, g_CurrentSequence->timeDivision, SEQUENCER_UPDATE_HZ);
    }

}

volatile static BOOL bEventSent=FALSE;
volatile static BOOL bSend=FALSE;
volatile static BOOL bEOTflag=FALSE;
volatile static BOOL bStopped=FALSE;
volatile static BOOL endOfSequence=FALSE;
volatile static U32 TimeAdd=0;
volatile static evntFuncPtr myFunc=NULL;
volatile static U32 currentDelta=0;
volatile static const sEventList *pCurrentEvent=0;
volatile static U32 timeElapsed=0;
volatile static sTrackState_t *pActiveTrackState=0;
volatile static sTrack_t *pTrack=0;

#define MIDI_DELTA_MARGIN 1


// single track handler
void updateStep(){
 bStopped=FALSE;

 if(g_CurrentSequence==0) return;

 //get track, there is only one active
 pTrack=g_CurrentSequence->arTracks[0];
 pActiveTrackState=&(pTrack->currentState);

 //check sequence state if paused do nothing
  if(pActiveTrackState->playState==PS_PAUSED) {
    am_allNotesOff(16);
    return;
  }

  switch(pActiveTrackState->playState){
    case PS_PLAYING:{
      bStopped=FALSE;
    }break;
    case PS_STOPPED:{
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
                pActiveTrackState->currentSeqPos=0L;
                pActiveTrackState->timeElapsedInt=0L;
                pActiveTrackState->currentSeqPos=0;
           }

          g_CurrentSequence->timeElapsedFrac=0L;
          g_CurrentSequence->timeStep=0L;

          //reset tempo to initial valueas taken during start(get them from main sequence?)
          g_CurrentSequence->timeStep=am_calculateTimeStep(pActiveTrackState->currentBPM,g_CurrentSequence->timeDivision, SEQUENCER_UPDATE_HZ);
#ifdef IKBD_MIDI_SEND_DIRECT
        clearMidiOutputBuffer();
#endif
          return;
      }else{
          //do nothing
          return;
      }

    }break;
  };

  bStopped=FALSE; //we replaying, so we have to reset this flag

  if(pActiveTrackState->bTempoChanged!=FALSE){
    pActiveTrackState->currentBPM=60000000/pActiveTrackState->currentTempo;
    g_CurrentSequence->timeStep=am_calculateTimeStep(pActiveTrackState->currentBPM, g_CurrentSequence->timeDivision, SEQUENCER_UPDATE_HZ);
    pActiveTrackState->bTempoChanged=FALSE;

    amTrace("Handle tempo change new qn: %lu timestep:%lu\n",pActiveTrackState->currentTempo,g_CurrentSequence->timeStep);
  }

   g_CurrentSequence->timeElapsedFrac += g_CurrentSequence->timeStep;
   TimeAdd = g_CurrentSequence->timeElapsedFrac >> 16;
   g_CurrentSequence->timeElapsedFrac &= 0xffff;

   pCurrentEvent=pActiveTrackState->currEventPtr;

   bEOTflag=isEOT(pActiveTrackState->currEventPtr);
   timeElapsed=pActiveTrackState->timeElapsedInt;
//reset
   myFunc=NULL;
   bEventSent=FALSE;
   bSend=FALSE;

   currentDelta=pCurrentEvent->eventBlock.uiDeltaTime;

   if(currentDelta==timeElapsed){
        bSend=TRUE;
   }else if(currentDelta==timeElapsed-MIDI_DELTA_MARGIN){
        bSend=TRUE;
   }

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
    while(bEOTflag!=FALSE&&pCurrentEvent->eventBlock.uiDeltaTime==0){
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
        //++pActiveTrackState->currentSeqPos;

        if(pCurrentEvent) bEOTflag=isEOT(pCurrentEvent);
    }

    if(bEOTflag!=FALSE){
      endOfSequence=TRUE;
    }

    bEventSent=TRUE;
   }
  } //endif

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

// multitrack with subsong versions
// multitrack
void updateStepMulti(){
 BOOL endOfSequence=FALSE;
 static BOOL bStopped=FALSE;
 sTrackState_t *pActiveTrackState=0;
 sTrack_t *pTrack=0;

 if(g_CurrentSequence==0) return;

 U8 activeTrack=g_CurrentSequence->ubActiveTrack;
 pTrack=g_CurrentSequence->arTracks[activeTrack];

 if(pTrack==0) return; //add assert ?

 pActiveTrackState=&(pTrack->currentState);
  
 //check sequence state if paused do nothing
  if(pActiveTrackState->playState==PS_PAUSED) {
    if(midiOutEnabled!=FALSE) am_allNotesOff(16);
    return;
  }
  
  switch(pActiveTrackState->playState){
    case PS_PLAYING:{
      bStopped=FALSE;
    }break;
    case PS_STOPPED:{
      //check sequence state if stopped reset position on all tracks
      //and reset tempo to default, but only once

      if(bStopped==FALSE){
          bStopped=TRUE;
          pActiveTrackState=0;
          pTrack=0;

          //reset each track
          for (int i=0;i<g_CurrentSequence->ubNumTracks;++i){
            pTrack=g_CurrentSequence->arTracks[i];

            if(pTrack){
                pActiveTrackState=&(pTrack->currentState);
                pActiveTrackState->currentTempo=DEFAULT_MPQN;
                pActiveTrackState->currentBPM=DEFAULT_BPM;
                pActiveTrackState->currentSeqPos=0L;
                pActiveTrackState->timeElapsedInt=0L;
                pActiveTrackState->currentSeqPos=0;
            }

          }

          g_CurrentSequence->timeElapsedFrac=0L;
          g_CurrentSequence->timeStep=0L;

#ifdef IKBD_MIDI_SEND_DIRECT
        clearMidiOutputBuffer();
#endif

          //reset tempo to initial valueas taken during start(get them from main sequence?)
          g_CurrentSequence->timeStep=am_calculateTimeStep(pActiveTrackState->currentBPM,g_CurrentSequence->timeDivision, SEQUENCER_UPDATE_HZ);
          return;
      }else{
          //do nothing
          return;
      }

    }break;
  };

  bStopped=FALSE; //we replaying, so we have to reset this flag
  
  if(pActiveTrackState->bTempoChanged!=FALSE){

    pActiveTrackState->currentBPM=60000000/pActiveTrackState->currentTempo;
    g_CurrentSequence->timeStep=am_calculateTimeStep(pActiveTrackState->currentBPM, g_CurrentSequence->timeDivision, SEQUENCER_UPDATE_HZ);

    amTrace("Handle tempo change new qn: %lu timestep:%lu\n",pActiveTrackState->currentTempo,g_CurrentSequence->timeStep);
    pActiveTrackState->bTempoChanged=FALSE;
  }
  
   g_CurrentSequence->timeElapsedFrac += g_CurrentSequence->timeStep;
   U32 TimeAdd = g_CurrentSequence->timeElapsedFrac >> 16;
   g_CurrentSequence->timeElapsedFrac &= 0xffff;
   

  // repeat for each track
  for (int i=0;i<g_CurrentSequence->ubNumTracks;++i){
    //for each active track
      U32 count=0;
      sEventList *pEvent=0;
      sTrack_t *pTrack=g_CurrentSequence->arTracks[i];

      count=pTrack->currentState.currentSeqPos;
      pEvent=&(pTrack->pTrkEventList[count]);
      pTrack->currentState.timeElapsedInt+=TimeAdd;

        while((isEOT(pEvent)==FALSE)&&((pEvent->eventBlock.uiDeltaTime <= pTrack->currentState.timeElapsedInt)||pEvent->eventBlock.uiDeltaTime==0)){
             endOfSequence=FALSE;
             pTrack->currentState.timeElapsedInt -= pEvent->eventBlock.uiDeltaTime;

             if(pTrack->currentState.bMute==FALSE){
               //play note
                evntFuncPtr myFunc=NULL;
#ifdef IKBD_MIDI_SEND_DIRECT
                //execute callback which copies data to midi buffer (_MIDIsendBuffer)
                amTrace("Send..\n");
                myFunc=pEvent->eventBlock.copyEventCb.func;

                 printEventBlock(&pEvent->eventBlock);
                 (*myFunc)((void *)pEvent->eventBlock.dataPtr);
#else
                amTrace("Send..\n");

                //execute callback which sends data directly to midi out
                myFunc= pEvent->eventBlock.sendEventCb.func;
                printEventBlock(&pEvent->eventBlock);
               (*myFunc)((void *)pEvent->eventBlock.dataPtr);
#endif

               ++count;
               pEvent=&(pTrack->pTrkEventList[count]);
             }else{
               //silence whole channel / MUTE channel
               U8 ch = getChannelNbFromEventBlock(&pEvent->eventBlock);
               if(ch!=127)  all_notes_off(ch);

                if(isEOT(pEvent)==FALSE){
                 ++count;
                 pEvent=&(pTrack->pTrkEventList[count]);
               }
             } //end mute
         }

         //check for end of sequence
         if(isEOT(pEvent)){
           endOfSequence=TRUE;
         }

      g_CurrentSequence->arTracks[i]->currentState.currentSeqPos=count;
  } // end track processing
  
  //check if we have end of sequence
  //on all tracks
  if(endOfSequence!=FALSE){
    onEndSequence();
    endOfSequence=FALSE;
    amTrace("End of Sequence\n");
  }
}

//replay control
BOOL isSeqPlaying(void){
  sTrack_t *pTrack=0;

  if(g_CurrentSequence!=0){
    U8 activeTrack=g_CurrentSequence->ubActiveTrack;
    pTrack=g_CurrentSequence->arTracks[activeTrack];

    if(pTrack){
        if(pTrack->currentState.playState==PS_PLAYING)
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
    U8 activeTrack=g_CurrentSequence->ubActiveTrack;
    pTrack=g_CurrentSequence->arTracks[activeTrack];

    if(pTrack){
        if(pTrack->currentState.playState!=PS_STOPPED){
          pTrack->currentState.playState=PS_STOPPED;
          printf("Stop sequence\n");
        }
    }
  }

  //all notes off
  am_allNotesOff(15);
}

void pauseSeq(){
  sTrack_t *pTrack=0;
  U8 activeTrack=0;

  //printf("Pause/Resume.\n");
  if(g_CurrentSequence!=0){
    // TODO: handling individual tracks for MIDI 2 type
    // for one sequence(single/multichannel) we will check state of the first track only
    activeTrack=g_CurrentSequence->ubActiveTrack;
    pTrack=g_CurrentSequence->arTracks[activeTrack];
    
    if(pTrack){
        switch(pTrack->currentState.playState){
            case PS_PLAYING:{
                pTrack->currentState.playState=PS_PAUSED;
                printf("Pause sequence\n");
            }break;
            case PS_PAUSED:{
                pTrack->currentState.playState=PS_PLAYING;
            }break;
        };
    }
  }
  //all notes off
  am_allNotesOff(15);
}//pauseSeq

void playSeq(void){

 if(g_CurrentSequence!=0){
    //set state
    U8 activeTrack=g_CurrentSequence->ubActiveTrack;
    sTrack_t *pTrack=g_CurrentSequence->arTracks[activeTrack];

    if(pTrack){
        if(pTrack->currentState.playState==PS_STOPPED) {
            pTrack->currentState.playState=PS_PLAYING;
            printf("Play sequence\n");
        }
    }

  }
}

void muteTrack(const U16 trackNb,const BOOL bMute){
  if(((g_CurrentSequence!=0)&&(trackNb<AMIDI_MAX_TRACKS))){
    g_CurrentSequence->arTracks[trackNb]->currentState.bMute=bMute;
    printf("Mute track %d\n",trackNb);
  }
}

void toggleReplayMode(void){
  U8 activeTrack=0;
  sTrack_t *pTrack=0;

  if(g_CurrentSequence!=0){
    activeTrack=g_CurrentSequence->ubActiveTrack;
    pTrack=g_CurrentSequence->arTracks[activeTrack];

    if(pTrack){
        switch(pTrack->currentState.playMode){
          case S_PLAY_ONCE:{
               pTrack->currentState.playMode=S_PLAY_LOOP;
          }break;
          case S_PLAY_LOOP:{
                pTrack->currentState.playMode=S_PLAY_ONCE;
          }break;
        }
    }
  }
}

void printSequenceState(){

if(g_CurrentSequence){

    printf("Td/PPQN: %u\n",g_CurrentSequence->timeDivision);

    printf("Time step: %lu\n",g_CurrentSequence->timeStep);
    printf("Time elapsedFrac: %lu\n",g_CurrentSequence->timeElapsedFrac);
    printf("EOT threshold: %lu\n",g_CurrentSequence->eotThreshold);

    sTrack_t *pTrack=0;

    switch(g_CurrentSequence->seqType){

    case ST_SINGLE:{
      pTrack=g_CurrentSequence->arTracks[0];
      sTrackState_t *pTrackState=0;

      if(pTrack){
         printf("Track state:\n");
         pTrackState=&(pTrack->currentState);
         printf("\tTime elapsed: %lu\n",pTrackState->timeElapsedInt);
         printf("\tCur BPM: %lu\n",pTrackState->currentBPM);
         printf("\tCur SeqPos: %lu\n",pTrackState->currentSeqPos);
         printf("\tCur Tempo: %lu\n",pTrackState->currentTempo);
         printf("\tPlay mode: %s\n",getPlayModeStr(pTrackState->playMode));
         printf("\tPlay state: %s\n",getPlayStateStr(pTrackState->playState));
         printf("\tMute: %d\n",pTrackState->bMute);
      }
    }break;
    case ST_MULTI:{
        pTrack=g_CurrentSequence->arTracks[g_CurrentSequence->ubActiveTrack];
        sTrackState_t *pTrackState=&(pTrack->currentState);

        printf("Nb of tracks: %d\n",g_CurrentSequence->ubNumTracks);
        printf("Active track: %d\n",g_CurrentSequence->ubActiveTrack);
        printf("Play mode: %s\n",getPlayModeStr(pTrackState->playMode));
        printf("Play state: %s\n",getPlayStateStr(pTrackState->playState));
        printf("Cur Tempo: %lu\n",pTrackState->currentTempo);
        printf("Cur BPM: %lu\n",pTrackState->currentBPM);

        for (int i=0;i<g_CurrentSequence->ubNumTracks;++i){
            pTrack=g_CurrentSequence->arTracks[i];
            pTrackState=0;

            if(pTrack){
                printf("Track[%d]\t",i);
                pTrackState=&(pTrack->currentState);
                printf("\tTime elapsed: %lu\t",pTrackState->timeElapsedInt);
                printf("\tCur SeqPos: %lu\t",pTrackState->currentSeqPos);
                printf("\tMute: %d\n",pTrackState->bMute);
            }
        }
    }break;
    case ST_MULTI_SUB:{
        //TODO:
        printf("Nb of tracks: %d\n",g_CurrentSequence->ubNumTracks);
        printf("Active track: %d\n",g_CurrentSequence->ubActiveTrack);

        for (int i=0;i<g_CurrentSequence->ubNumTracks;++i){
            pTrack=g_CurrentSequence->arTracks[i];
            sTrackState_t *pTrackState=0;

            if(pTrack){
                printf("Track[%d]\n",i);
                pTrackState=&(pTrack->currentState);
                printf("Time elapsed: %lu\n",pTrackState->timeElapsedInt);
                printf("Cur BPM: %lu\n",pTrackState->currentBPM);
                printf("Cur SeqPos: %lu\n",pTrackState->currentSeqPos);
                printf("Cur Tempo: %lu\n",pTrackState->currentTempo);
                printf("Play mode: %s\n",getPlayModeStr(pTrackState->playMode));
                printf("Play state: %s\n",getPlayStateStr(pTrackState->playState));
                printf("Mute: %d\n",pTrackState->bMute);
            }
        }

    }break;

    };


 }

 printMidiSendBufferState();
}

void printMidiSendBufferState(){
    amTrace("Midi send buffer bytes to send: %d\n",MIDIbytesToSend);

    if(MIDIbytesToSend>0){
        for(int i=0;i<MIDIbytesToSend;++i){
            amTrace("%x",MIDIsendBuffer[i]);
        }

        amTrace(".\n");
    }

}

const U8 *getPlayStateStr(const ePlayState state){

    switch(state){
        case PS_STOPPED:
            return "Stopped";
        break;
        case PS_PLAYING:
            return "Playing";
        break;
        case PS_PAUSED:
            return "Paused";
        break;
        default:
            return NULL;
    }
}

const U8 *getPlayModeStr(const ePlayMode mode){
    switch(mode){
        case S_PLAY_ONCE:
            return "Play once";
        break;
        case S_PLAY_LOOP:
            return "Loop";
        break;
        case S_PLAY_RANDOM:
            return "Random";
        break;
        default:
            return NULL;
    }
}
