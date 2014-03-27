
#include "config.h"
#include "timing/mfp.h"
#include "amidilib.h"
#include "midi_send.h"

#include "timing/miditim.h"
#include "nkt.h"

//nkt replay
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
        g_CurrentNktSequence->currentBlock=0;

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
    seq->playState = getGlobalConfig()->playState;
    seq->playMode = getGlobalConfig()->playMode;
    seq->timeElapsedFrac=0L;
    seq->timeStep=0L;
    seq->timeStep=am_calculateTimeStep(seq->currentBPM, seq->timeDivision, SEQUENCER_UPDATE_HZ);

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


