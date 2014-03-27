
#include "config.h"
#include "timing/mfp.h"
#include "amidilib.h"
#include "midi_send.h"

#include "timing/miditim.h"
#include "nkt.h"

// nkt replay
extern void replayNktTC(void);

static sNktSeq *g_CurrentNktSequence=0;

void getCurrentNktSeq(sNktSeq **pSeq){
  *pSeq=g_CurrentNktSequence;
}

BOOL isEOT(volatile const sNktBlock_t *pPtr){
  if(pPtr->msgType==NKT_END) return TRUE;
  return FALSE;
}

void onEndSequence(){

if(g_CurrentNktSequence){

      if(g_CurrentNktSequence->playMode==S_PLAY_ONCE){
          //reset set state to stopped
          //reset song position on all tracks
          g_CurrentNktSequence->playState=PS_STOPPED;
        }else if(g_CurrentNktSequence->playMode==S_PLAY_LOOP){
          g_CurrentNktSequence->playState=PS_PLAYING;
        }

        am_allNotesOff(16);
        g_CurrentNktSequence->timeElapsedInt=0L;
        g_CurrentNktSequence->currentTempo=DEFAULT_MPQN;
        g_CurrentNktSequence->currentBPM=DEFAULT_BPM;
        g_CurrentNktSequence->currentBlockId=0;

#ifdef IKBD_MIDI_SEND_DIRECT
        flushMidiSendBuffer();
#endif
        // reset all tracks state
        g_CurrentNktSequence->timeElapsedFrac=0L;
        g_CurrentNktSequence->timeStep=am_calculateTimeStep(g_CurrentNktSequence->currentBPM, g_CurrentNktSequence->timeDivision, SEQUENCER_UPDATE_HZ);
    }
}


// init sequence
void initNktSeq(sNktSeq *seq){
 g_CurrentNktSequence=0;

if(seq!=0){
    U8 mode=0,data=0;
    g_CurrentNktSequence=seq;

    seq->currentTempo=DEFAULT_MPQN;
    seq->currentBPM=DEFAULT_BPM;
    seq->timeElapsedInt=0L;

    seq->timeElapsedFrac=0L;
    seq->timeStep=0L;
    seq->timeStep=am_calculateTimeStep(seq->currentBPM, seq->timeDivision, SEQUENCER_UPDATE_HZ);
    seq->playState = getGlobalConfig()->playState;
    seq->playMode = getGlobalConfig()->playMode;

#ifdef IKBD_MIDI_SEND_DIRECT
    flushMidiSendBuffer();
#endif

    getMFPTimerSettings(SEQUENCER_UPDATE_HZ,&mode,&data);

#ifdef DEBUG_BUILD
    amTrace("%dhz update interval, Time step: %d\r\n",SEQUENCER_UPDATE_HZ,seq->timeStep);
    amTrace("calculated mode: %d, data: %d\n",mode,data);
#endif

   installReplayRout(mode, data, replayNktTC);
  } //endif
 return;
}


volatile static BOOL bEventSent=FALSE;
volatile static BOOL bSend=FALSE;
volatile static BOOL bEOTflag=FALSE;
volatile static BOOL bStopped=FALSE;
volatile static BOOL endOfSequence=FALSE;
volatile static U32 TimeAdd=0;
volatile static U32 currentDelta=0;
volatile static const sEventList *pCurrentEvent=0;
volatile static U32 timeElapsed=0;
volatile static sNktBlock_t *nktBlk=0;

// single track handler
void updateStepNkt(){
 bStopped=FALSE;

 if(g_CurrentNktSequence==0) return;

 //check sequence state if paused do nothing
  if(g_CurrentNktSequence->playState==PS_PAUSED) {
    am_allNotesOff(16);

#ifdef IKBD_MIDI_SEND_DIRECT
    flushMidiSendBuffer();
#endif

    return;
  }

  switch(g_CurrentNktSequence->playState){
    case PS_PLAYING:{
      bStopped=FALSE;
    }break;
    case PS_STOPPED:{
      //check sequence state if stopped reset position on all tracks
      //and reset tempo to default, but only once

      if(bStopped==FALSE){
          bStopped=TRUE;

          g_CurrentNktSequence->currentTempo=DEFAULT_MPQN;
          g_CurrentNktSequence->currentBPM=DEFAULT_BPM;
          g_CurrentNktSequence->timeElapsedInt=0L;
          g_CurrentNktSequence->timeElapsedFrac=0L;
          g_CurrentNktSequence->timeStep=0L;

          //reset tempo to initial valueas taken during start(get them from main sequence?)
          g_CurrentNktSequence->timeStep=am_calculateTimeStep(g_CurrentNktSequence->currentBPM,g_CurrentNktSequence->timeDivision, SEQUENCER_UPDATE_HZ);

#ifdef IKBD_MIDI_SEND_DIRECT
           flushMidiSendBuffer();
#endif
          //rewind to the first event
          g_CurrentNktSequence->currentBlockId=0;
          return;
      }else{
          //do nothing
          return;
      }

    }break;
  };

   bStopped=FALSE; //we replaying, so we have to reset this flag
   nktBlk=&(g_CurrentNktSequence->pEvents[g_CurrentNktSequence->currentBlockId]);

   if(nktBlk) bEOTflag=isEOT(nktBlk);
   timeElapsed=g_CurrentNktSequence->timeElapsedInt;
//reset

   bEventSent=FALSE;
   bSend=FALSE;
   currentDelta=nktBlk->delta;

   if(currentDelta==timeElapsed) bSend=TRUE;

if(bEOTflag==FALSE&&bSend!=FALSE){
    endOfSequence=FALSE;

#ifdef IKBD_MIDI_SEND_DIRECT
    //copy event data to custom buffer
    MIDIbytesToSend+=nktBlk->blockSize;
    amMemCpy(MIDIsendBuffer,nktBlk->pData, nktBlk->blockSize);
#else
        //send to xbios
        amMidiSendData(nktBlk->blockSize,nktBlk->pData);
#endif

   //go to next event
   ++g_CurrentNktSequence->currentBlockId;
   nktBlk=&(g_CurrentNktSequence->pEvents[g_CurrentNktSequence->currentBlockId]);

   if(nktBlk) bEOTflag=isEOT(nktBlk);

   //check if next events are null and pack buffer until first next non zero delta
   while(bEOTflag!=FALSE&&pCurrentEvent->eventBlock.uiDeltaTime==0){
        //handle event
#ifdef IKBD_MIDI_SEND_DIRECT
       MIDIbytesToSend+=nktBlk->blockSize;
       amMemCpy(MIDIsendBuffer,nktBlk->pData, nktBlk->blockSize);
#else
         //send to xbios
        amMidiSendData(nktBlk->blockSize,nktBlk->pData);
#endif

       //go to next event
       ++g_CurrentNktSequence->currentBlockId;
       nktBlk=&(g_CurrentNktSequence->pEvents[g_CurrentNktSequence->currentBlockId]);
       if(nktBlk) bEOTflag=isEOT(nktBlk);
    }

    if(bEOTflag!=FALSE){
      endOfSequence=TRUE;
    }

    bEventSent=TRUE;

  } //endif

    g_CurrentNktSequence->timeElapsedFrac += g_CurrentNktSequence->timeStep;
    TimeAdd = g_CurrentNktSequence->timeElapsedFrac >> 16;
    g_CurrentNktSequence->timeElapsedFrac &= 0xffff;

    if(TimeAdd>1)TimeAdd=1;


   //add time elapsed
   if(bEventSent!=FALSE){
     g_CurrentNktSequence->timeElapsedInt=0;
   }else{
     g_CurrentNktSequence->timeElapsedInt=g_CurrentNktSequence->timeElapsedInt+TimeAdd;
   }

  //check if we have end of sequence
  //on all tracks
  if(endOfSequence!=FALSE){
    onEndSequence();
    endOfSequence=FALSE;
    amTrace("End of Sequence\n");
  }

} //end UpdateStep()



