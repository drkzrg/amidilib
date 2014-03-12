
//tos version
#include <lzo/lzoconf.h>
#include <math.h>

#include "timing/mfp.h"
#include "amidilib.h"
#include "midi_send.h"
#include "config.h"
#include "timing/miditim.h"
#include "list/list.h"

extern void midiSeqReplay(void);


//extern U8 MIDIsendBuffer[32*1024]; //buffer from which we will send all data from the events once per frame
//extern U16 MIDIbytesToSend;

extern volatile BOOL midiOutEnabled;
extern volatile BOOL ymOutEnabled;

volatile static BOOL handleTempoChange=FALSE;
volatile static BOOL bTempoChanged=FALSE;

sSequence_t *g_CurrentSequence=0;

void getCurrentSeq(sSequence_t **pSeq){
  *pSeq=g_CurrentSequence;
}

void initSeq(sSequence_t *seq){
    g_CurrentSequence=0;


 if(seq!=0){

    U8 mode=0,data=0;
    sTrack_t *pTrack=0;
    sTrackState_t *pTrackState=0;
    U8 activeTrack=seq->ubActiveTrack;

    g_CurrentSequence=seq;

    MIDIbytesToSend=0;
    //MIDIbufferReady=0;

    for(int i=0;i<seq->ubNumTracks;++i){
        pTrack=seq->arTracks[i];

        if(pTrack){
            pTrackState=&(pTrack->currentState);
            pTrackState->currentPPQN=DEFAULT_PPQN;
            pTrackState->currentTempo=DEFAULT_MPQN;
            pTrackState->currentBPM=DEFAULT_BPM;
            pTrackState->currentSeqPos=0L;
            pTrackState->timeElapsedInt=0L;
            pTrackState->bMute=FALSE;
            pTrackState->currentBPM=60000000/DEFAULT_MPQN;
            pTrackState->playState = getGlobalConfig()->playState;
            pTrackState->playMode = getGlobalConfig()->playMode;
        }

    } 
  
    seq->timeElapsedFrac=0L;
    seq->timeStep=0L;
    
    seq->timeStep=am_calculateTimeStep(DEFAULT_BPM, DEFAULT_PPQN, SEQUENCER_UPDATE_HZ);
 
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
         pTrackState->currentPPQN=DEFAULT_PPQN;
         pTrackState->currentTempo=DEFAULT_MPQN;
         pTrackState->currentBPM=DEFAULT_BPM;
         pTrackState->currentSeqPos=0L;
         pTrackState->timeElapsedInt=0L;
         pTrackState->bMute=FALSE;
         pTrackState->currentBPM=60000000/DEFAULT_MPQN;
         pTrackState->playState = getGlobalConfig()->playState;
         pTrackState->playMode = getGlobalConfig()->playMode;
     }
    } 
  
    seq->timeElapsedFrac=0L;
    seq->timeStep=0L;
    
    seq->timeStep=am_calculateTimeStep(DEFAULT_BPM, DEFAULT_PPQN, SEQUENCER_UPDATE_HZ);

  }
  
 return;
}

BOOL isEOT(const sEventList *pPtr){
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

        if(midiOutEnabled!=FALSE) am_allNotesOff(16);

        //reset all tracks state
        g_CurrentSequence->timeElapsedFrac=0L;
        g_CurrentSequence->timeStep=am_calculateTimeStep(DEFAULT_BPM, DEFAULT_PPQN, SEQUENCER_UPDATE_HZ);

        for (int i=0;i<g_CurrentSequence->ubNumTracks;++i){

          pTrack=g_CurrentSequence->arTracks[i];

          if(pTrack){
              pTrack->currentState.currentSeqPos=0L;
              pTrack->currentState.timeElapsedInt=0L;
              pTrack->currentState.currentPPQN=DEFAULT_PPQN;
              pTrack->currentState.currentTempo=DEFAULT_MPQN;
              pTrack->currentState.currentBPM=DEFAULT_BPM;
              pTrack->currentState.currentBPM=60000000/DEFAULT_MPQN;
          }

        }
    }

}

void updateStep(){
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
                pActiveTrackState->currentPPQN=DEFAULT_PPQN;
                pActiveTrackState->currentTempo=DEFAULT_MPQN;
                pActiveTrackState->currentBPM=DEFAULT_BPM;
                pActiveTrackState->currentSeqPos=0L;
                pActiveTrackState->timeElapsedInt=0L;
            }

          }

          g_CurrentSequence->timeElapsedFrac=0L;
          g_CurrentSequence->timeStep=0L;
          //reset tempo to initial valueas taken during start(get them from main sequence?)
          g_CurrentSequence->timeStep=am_calculateTimeStep(DEFAULT_BPM, DEFAULT_PPQN, SEQUENCER_UPDATE_HZ);
          return;
      }else{
          //do nothing
          return;
      }

    }break;

  }

  bStopped=FALSE; //we replaying, so we have to reset this flag
  
  if(handleTempoChange!=FALSE){
    pActiveTrackState->currentBPM=60000000/pActiveTrackState->currentTempo;
    g_CurrentSequence->timeStep=am_calculateTimeStep(pActiveTrackState->currentBPM, pActiveTrackState->currentPPQN, SEQUENCER_UPDATE_HZ);
    printf("Handle tempo change new qn: %d timestep:%d\n",pActiveTrackState->currentTempo,g_CurrentSequence->timeStep);

    handleTempoChange=FALSE;
  }
  
   g_CurrentSequence->timeElapsedFrac += g_CurrentSequence->timeStep;
   U32 TimeAdd = g_CurrentSequence->timeElapsedFrac >> 16;
   g_CurrentSequence->timeElapsedFrac &= 0xffff;
   
  //repeat for each track
  for (int i=0;i<g_CurrentSequence->ubNumTracks;++i){
    //for each active track
      U32 count=0;
      sEventList *pEvent=0;
      sTrack_t *pTrack=g_CurrentSequence->arTracks[i];

      if(pTrack!=0){
         count=pTrack->currentState.currentSeqPos;
         pEvent=&(pTrack->pTrkEventList[count]);
         pTrack->currentState.timeElapsedInt+=TimeAdd;

         if(pEvent){
             while( (isEOT(pEvent)==FALSE)&&pEvent->eventBlock.uiDeltaTime <= pTrack->currentState.timeElapsedInt){
             endOfSequence=FALSE;
             pTrack->currentState.timeElapsedInt -= pEvent->eventBlock.uiDeltaTime;

             if(pTrack->currentState.bMute==FALSE){
               //play note
                evntFuncPtr myFunc=NULL;
#ifdef IKBD_MIDI_SEND_DIRECT
                //execute callback which copies data to midi buffer (_MIDIsendBuffer)

                myFunc=pEvent->eventBlock.copyEventCb.func;
                if(myFunc){
                    printEventBlock(&pEvent->eventBlock);
                    (*myFunc)((void *)pEvent->eventBlock.dataPtr);
                }else{
                    printf("ERROR:function pointer NULL\n");
                }


                //TODO log midi out buffer state

#else
               //execute callback which sends data directly to midi out
                myFunc= pEvent->eventBlock.sendEventCb.func;
               (*myFunc)((void *)pEvent->eventBlock.dataPtr);
#endif

               ++count;
               pEvent=&(pTrack->pTrkEventList[count]);
             }else{
               //silence whole channel
               U8 ch = getChannelNbFromEventBlock(&pEvent->eventBlock);
               if(ch!=127)  all_notes_off(ch);

                if(isEOT(pEvent)==FALSE){
                 ++count;
                 pEvent=&(pTrack->pTrkEventList[count]);
               }
             }
         }

         }//end event null check

         //check for end of sequence
         if(isEOT(pEvent)){
           endOfSequence=TRUE;
         }
          g_CurrentSequence->arTracks[i]->currentState.currentSeqPos=count;
      }
  }
  
  //check if we have end of sequence
  //on all tracks
  if(endOfSequence!=FALSE){
    onEndSequence();
    endOfSequence=FALSE;
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
        }
    }


  }
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
            }break;
            case PS_PAUSED:{
                pTrack->currentState.playState=PS_PLAYING;
            }break;
        };
    }
  }
}//pauseSeq

void playSeq(void){

 if(g_CurrentSequence!=0){
    //set state
    U8 activeTrack=g_CurrentSequence->ubActiveTrack;
    sTrack_t *pTrack=g_CurrentSequence->arTracks[activeTrack];

    if(pTrack){
        if(pTrack->currentState.playState==PS_STOPPED) pTrack->currentState.playState=PS_PLAYING;
    }

  }
}

void muteTrack(U16 trackNb,BOOL bMute){
  if(((g_CurrentSequence!=0)&&(trackNb<AMIDI_MAX_TRACKS))){
    g_CurrentSequence->arTracks[trackNb]->currentState.bMute=bMute;
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
    printf("Nb of tracks: %d\n",g_CurrentSequence->ubNumTracks);
    printf("Active track: %d\n",g_CurrentSequence->ubActiveTrack);
    printf("Time step: %lu\n",g_CurrentSequence->timeStep);
    printf("Time elapsedFrac: %lu\n",g_CurrentSequence->timeElapsedFrac);
    printf("EOT threshold: %lu\n",g_CurrentSequence->eotThreshold);

    sTrack_t *pTrack=0;
    for (int i=0;i<g_CurrentSequence->ubNumTracks;++i){
        pTrack=g_CurrentSequence->arTracks[i];
        sTrackState_t *pTrackState=0;

        if(pTrack){
            printf("Track[%d]\n",i);
            pTrackState=&(pTrack->currentState);
            printf("Time elapsed: %d\n",pTrackState->timeElapsedInt);
            printf("Cur BPM: %d\n",pTrackState->currentBPM);
            printf("Cur PPQN: %lu\n",pTrackState->currentPPQN);
            printf("Cur SeqPos: %lu\n",pTrackState->currentSeqPos);
            printf("Cur Tempo: %lu\n",pTrackState->currentTempo);
            printf("Time signature: nn: %d\tdd: %d\tcc: %d\tbb: %d\r\n",pTrackState->timeSignature.nn,pTrackState->timeSignature.dd,pTrackState->timeSignature.cc,pTrackState->timeSignature.bb);
            printf("Play mode: %s\n",getPlayModeStr(pTrackState->playMode));
            printf("Play state: %s\n",getPlayStateStr(pTrackState->playState));
            printf("Mute: %d\n",pTrackState->bMute);
        }
    }
 }

 printMidiSendBufferState();
}

void printMidiSendBufferState(){

    printf("Midi send buffer bytes to send: %d\n",MIDIbytesToSend);
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
