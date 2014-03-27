
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
    clearMidiOutputBuffer();
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


