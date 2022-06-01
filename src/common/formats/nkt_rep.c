
/**  Copyright 2007-2021 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#include "nkt.h"
#include "nkt_util.h"

#include "memory.h"
#include "core/logger.h"

#include "timing/mfp.h"
#include "timing/miditim.h"

#include "midi.h"

#include "midi_cmd.h"
#include "rol_ptch.h"

#include "minilzo.h" //lzo pack / depack

#include "core/amprintf.h"

#ifdef ENABLE_GEMDOS_IO
#include <mint/ostruct.h>
#include <mint/osbind.h>
#include "gemdosio.h"
#else
#include <stdio.h>
#endif

// helper function for determining amount of memory we need for data / events buffers
AM_EXTERN int16 collectMidiTrackInfo(void *pMidiData, uint16 trackNb, sMidiTrackInfo_t *pBufInfo, Bool *bEOT);
AM_EXTERN uint16 isMultitrackReplay;

void setNktHeader(sNktHd* header, const sNktSeq *pNktSeq);
void setNktTrackInfo(sNktTrackInfo* header, const sNktSeq *pNktSeq);

static sNktSeq *g_CurrentNktSequence=0;

sNktSeq* const getActiveNktSequence(void)
{
  AssertMsg(g_CurrentNktSequence!=0,"Fatal error: Current nkt sequence is NULL.");
  return g_CurrentNktSequence;
}

static void resetMidiDevice(void)
{
    amAllNotesOff(16);

    // reset all controllers
    for(uint8 i=0;i<16;++i)
    {
      reset_all_controllers(i);
      omni_off(i);
    }

    #ifdef IKBD_MIDI_SEND_DIRECT
      Supexec(flushMidiSendBuffer);
    #endif
}

// this is called when sequence ends
static void onEndSequence(void)
{

  AssertMsg(g_CurrentNktSequence!=0,"Fatal error: Current nkt sequence is NULL.");
  
  if(g_CurrentNktSequence->sequenceState&NKT_PLAY_ONCE)
  {
    // set state to stopped
    // reset song position on all tracks
    g_CurrentNktSequence->sequenceState&=(uint16)(~(NKT_PS_PLAYING|NKT_PS_PAUSED));
  }
  else
  {
    // loop
    g_CurrentNktSequence->sequenceState&=(uint16)(~NKT_PS_PAUSED);
    g_CurrentNktSequence->sequenceState|=(uint16)NKT_PS_PLAYING;
  }

  // reset all tracks state

  for(uint16 i=0;i<g_CurrentNktSequence->nbOfTracks;++i)
  {
    g_CurrentNktSequence->pTracks[i].timeElapsedInt=0L;
    g_CurrentNktSequence->pTracks[i].timeElapsedFrac=0L;
    g_CurrentNktSequence->pTracks[i].currentBlockId=0L;
    g_CurrentNktSequence->pTracks[i].eventsBlockOffset=0L;
  }

  g_CurrentNktSequence->currentTempo.tempo=g_CurrentNktSequence->defaultTempo.tempo;
  g_CurrentNktSequence->timeStep=g_CurrentNktSequence->defaultTempo.tuTable[g_CurrentNktSequence->currentUpdateFreq];

  resetMidiDevice();

}

// init sequence
void initNktSequence(sNktSeq *pSeq, uint16 initialState, Bool bInstallUpdate){
 g_CurrentNktSequence=0;

if(pSeq!=0){
    g_CurrentNktSequence=pSeq;

    pSeq->currentUpdateFreq=NKT_U200HZ;
    pSeq->currentTempo.tempo = DEFAULT_MPQN;

    uint32 td=pSeq->timeDivision;
    uint32 bpm = DEFAULT_BPM;
    uint32 tempPPU = bpm * td;

    // precalc tempo table
    amTrace("Precalculating update step for Td: %d, Bpm: %d"NL,td,bpm);
    
    // precalculate values for different update steps
    if(tempPPU<65536)
    {
      const uint32 div = (tempPPU*65536)/60;
      pSeq->defaultTempo.tuTable[NKT_U25HZ] = pSeq->currentTempo.tuTable[NKT_U25HZ] = div/25;
      pSeq->defaultTempo.tuTable[NKT_U50HZ] = pSeq->currentTempo.tuTable[NKT_U50HZ] = div/50;
      pSeq->defaultTempo.tuTable[NKT_U100HZ] = pSeq->currentTempo.tuTable[NKT_U100HZ] = div/100;
      pSeq->defaultTempo.tuTable[NKT_U200HZ] = pSeq->currentTempo.tuTable[NKT_U200HZ] = div/200;
    }
    else
    {
      const uint32 div = ((tempPPU/60)*65536);
      pSeq->defaultTempo.tuTable[NKT_U25HZ] = pSeq->currentTempo.tuTable[NKT_U25HZ] = div/25;
      pSeq->defaultTempo.tuTable[NKT_U50HZ] = pSeq->currentTempo.tuTable[NKT_U50HZ] = div/50;
      pSeq->defaultTempo.tuTable[NKT_U100HZ] = pSeq->currentTempo.tuTable[NKT_U100HZ] = div/100;
      pSeq->defaultTempo.tuTable[NKT_U200HZ] = pSeq->currentTempo.tuTable[NKT_U200HZ] = div/200;
    }

    const uint32 up25Hz = pSeq->currentTempo.tuTable[NKT_U25HZ];
    const uint32 up50Hz = pSeq->currentTempo.tuTable[NKT_U25HZ];
    const uint32 up100Hz = pSeq->currentTempo.tuTable[NKT_U100HZ];
    const uint32 up200Hz = pSeq->currentTempo.tuTable[NKT_U200HZ];

    amTrace("Update 25hz: %d  [0x%x]"NL,up25Hz,up25Hz);
    amTrace("Update 50hz: %d [0x%x]"NL,up50Hz,up50Hz);
    amTrace("Update 100hz: %d [0x%x]"NL,up100Hz,up100Hz);
    amTrace("Update 200hz: %d [0x%x]"NL,up200Hz,up200Hz);

    for(uint16 i=0;i<pSeq->nbOfTracks;++i)
    {
        pSeq->pTracks[i].timeElapsedInt=0UL;
        pSeq->pTracks[i].timeElapsedFrac=0UL;
        pSeq->pTracks[i].currentBlockId=0UL;
        pSeq->pTracks[i].eventsBlockOffset=0L;
    }

    pSeq->timeStep=pSeq->defaultTempo.tuTable[pSeq->currentUpdateFreq];

    pSeq->sequenceState = initialState;


#ifdef IKBD_MIDI_SEND_DIRECT
    Supexec(clearMidiOutputBuffer);
#endif

    // install our interrupt handler
    if(bInstallUpdate==TRUE)
    {
        if(pSeq->nbOfTracks==1){
             amTrace("Setting single track replay"NL,0);
            isMultitrackReplay=0;
            Supexec(NktInstallReplayRout);
        }else{
            amTrace("Setting multitrack replay "NL,0);
            isMultitrackReplay=1;
            Supexec(NktInstallReplayRout);
        }

    }else{
            if(pSeq->nbOfTracks==1){
                amTrace("Setting single track replay"NL,0);
                isMultitrackReplay=0;
                Supexec(NktInstallReplayRoutNoTimers);
            }else{
                amTrace("Setting multitrack replay "NL,0);
                isMultitrackReplay=1;
                Supexec(NktInstallReplayRoutNoTimers);
            }
      }

#ifdef DEBUG_BUILD
  printNktSequenceState();
#endif

    setMT32Message("Ready ...");

  } //endif
 return;
}

#ifdef DEBUG_BUILD
void initNktSequenceManual(sNktSeq *pSeq, const uint16 state)
{
 g_CurrentNktSequence=0;
 AssertMsg(pSeq!=0,"Sequence is NULL");
 
  uint8 mode=0,data=0;
  g_CurrentNktSequence=pSeq;

  pSeq->currentTempo.tempo = pSeq->defaultTempo.tempo;

  for(uint16 i=0;i<pSeq->nbOfTracks;++i){
      pSeq->pTracks[i].timeElapsedInt = 0UL;
      pSeq->pTracks[i].timeElapsedFrac = 0UL;
      pSeq->pTracks[i].currentBlockId=0;
      pSeq->pTracks[i].eventsBlockOffset=0L;
  }

  pSeq->timeStep = pSeq->currentTempo.tuTable[pSeq->currentUpdateFreq];
  pSeq->sequenceState = state;

  #ifdef IKBD_MIDI_SEND_DIRECT
   Supexec(clearMidiOutputBuffer);
  #endif

  getMFPTimerSettings(SEQUENCER_UPDATE_HZ,&mode,&data);

  amTrace("%dhz update interval, Time step: %d\r"NL,SEQUENCER_UPDATE_HZ,pSeq->timeStep);
  amTrace("calculated mode: %d, data: %d"NL,mode,data);

  //printNktSequenceState();

 return;
}
#endif

volatile static Bool bStopped=FALSE;
volatile static Bool bPaused=FALSE;
volatile static uint32 TimeAdd=0;
volatile static uint32 addr;
volatile static sNktBlock *nktBlk=0;
volatile static uint16 TrackEndCount=0;

volatile uint8 requestedMasterVolume;
volatile uint8 requestedMasterBalance;
volatile static uint16 sequenceState;

volatile sMidiModuleSettings moduleSettings;
uint8 mt32TextMsg[20];

enum{
  IDX_VENDOR=1,
  IDX_DEVICE_ID=2,
  IDX_MODEL_ID=3,
  IDX_CMD_ID=4,
  IDX_MASTER_VOL=8,
  IDX_MASTER_PAN=8
};

static sSysEX_t arSetMasterVolumeGM   =  {11,(uint8 []){0xf0,0x00,0x00,0x00,0x00,0x40,0x00,0x04,0x7f,0x00,0xf7}};
static sSysEX_t arSetMasterBalanceGM  =  {11,(uint8 []){0xf0,0x00,0x00,0x00,0x00,0x40,0x00,0x06,0x7f,0x00,0xf7}};
static sSysEX_t arSetMasterVolumeMT32 =  {11,(uint8 []){0xf0,0x00,0x00,0x00,0x00,0x10,0x00,0x16,0x7f,0x00,0xf7}};
static sSysEX_t arSetTextMT32         =  {30,(uint8 []){0xf0,0x41,0x10,0x16,0x12,0x20,0x00,0x00,0x00,0x00,0x00,
                                                              0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                                              0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xf7}};

static AM_INLINE void handleMasterSettings(void) AM_FORCE_INLINE;

static AM_INLINE void handleMasterSettings(void) 
{
    // handle volume/balance/reverb change
   if(moduleSettings.masterVolume!=requestedMasterVolume)
   {

       if(MT32_MODEL_ID==moduleSettings.modelID)
       {

            //handle mt32 volume
            if(requestedMasterVolume<=MIDI_MASTER_VOL_MAX_MT32){

                arSetMasterVolumeMT32.data[IDX_VENDOR]=moduleSettings.vendorID;
                arSetMasterVolumeMT32.data[IDX_DEVICE_ID]=moduleSettings.deviceID;
                arSetMasterVolumeMT32.data[IDX_MODEL_ID]=moduleSettings.modelID;

                arSetMasterVolumeMT32.data[IDX_CMD_ID]=0x12;                          // sending
                arSetMasterVolumeMT32.data[IDX_MASTER_VOL]=requestedMasterVolume;
                arSetMasterVolumeMT32.data[9]=amCalcRolandChecksum(&arSetMasterVolumeMT32.data[5],&arSetMasterVolumeMT32.data[8]);

                sendSysEXNoLog(&arSetMasterVolumeMT32);

#ifdef IKBD_MIDI_SEND_DIRECT
                Supexec(flushMidiSendBuffer);
#endif

                moduleSettings.masterVolume=requestedMasterVolume;
            }

            if(mt32TextMsg[0]!=0){

                amMemSet((void *)&(arSetTextMT32.data[8]),0,sizeof(uint8)*20);

                arSetTextMT32.data[IDX_VENDOR]=moduleSettings.vendorID;
                arSetTextMT32.data[IDX_DEVICE_ID]=moduleSettings.deviceID;
                arSetTextMT32.data[IDX_MODEL_ID]=moduleSettings.modelID;
                arSetTextMT32.data[IDX_CMD_ID]=0x12;

                amMemCpy(&arSetTextMT32.data[8],&mt32TextMsg[0],sizeof(uint8)*20);
                arSetTextMT32.data[28]=amCalcRolandChecksum(&arSetTextMT32.data[5],&arSetTextMT32.data[27]);

                // update text
                sendSysEXNoLog(&arSetTextMT32);

#ifdef IKBD_MIDI_SEND_DIRECT
                Supexec(flushMidiSendBuffer);
#endif

                // reset text
                amMemSet(&mt32TextMsg[0],0,sizeof(uint8)*20);
            }


            // todo reverb change request

            // update text

       }else{
        // General Midi GS, todo move it to configurable callback
            // send new master vol
            if(requestedMasterVolume<=MIDI_MASTER_VOL_MAX_GM){

                arSetMasterVolumeGM.data[IDX_VENDOR]=moduleSettings.vendorID;
                arSetMasterVolumeGM.data[IDX_DEVICE_ID]=moduleSettings.deviceID;
                arSetMasterVolumeGM.data[IDX_MODEL_ID]=moduleSettings.modelID;

                arSetMasterVolumeGM.data[IDX_CMD_ID]=0x12;                         // sending
                arSetMasterVolumeGM.data[IDX_MASTER_VOL]=requestedMasterVolume;
                arSetMasterVolumeGM.data[9]=amCalcRolandChecksum(&arSetMasterVolumeGM.data[5],&arSetMasterVolumeGM.data[8]);

                sendSysEXNoLog(&arSetMasterVolumeGM);

#ifdef IKBD_MIDI_SEND_DIRECT
                Supexec(flushMidiSendBuffer);
#endif

                moduleSettings.masterVolume=requestedMasterVolume;

            }
         }

         if(moduleSettings.masterBalance!=requestedMasterBalance){

             // send new balance
            arSetMasterBalanceGM.data[IDX_VENDOR]=moduleSettings.vendorID;
            arSetMasterBalanceGM.data[IDX_DEVICE_ID]=moduleSettings.deviceID;
            arSetMasterBalanceGM.data[IDX_MODEL_ID]=moduleSettings.modelID;
            arSetMasterBalanceGM.data[IDX_CMD_ID]=0x12;                                // sending
            arSetMasterBalanceGM.data[IDX_MASTER_PAN]=requestedMasterBalance;
            arSetMasterBalanceGM.data[9]=amCalcRolandChecksum(&arSetMasterBalanceGM.data[5],&arSetMasterBalanceGM.data[8]);

            sendSysEXNoLog(&arSetMasterBalanceGM);

#ifdef IKBD_MIDI_SEND_DIRECT
            Supexec(flushMidiSendBuffer);
#endif

            moduleSettings.masterBalance=requestedMasterBalance;
         }
    }

}

//update step for single track replay

void updateStepNkt(void)
{
 // handle master volume, balance, reverb, mt32 text
 handleMasterSettings();

 if(g_CurrentNktSequence==0) return;

 sequenceState=g_CurrentNktSequence->sequenceState;

 //check sequence state if paused do nothing
 if((sequenceState&NKT_PS_PAUSED))
 {
     if(bPaused!=TRUE)
     {
        bPaused=TRUE;
        bStopped=FALSE;

        // all notes off but only once
        amAllNotesOff(16);

        #ifdef IKBD_MIDI_SEND_DIRECT
             Supexec(flushMidiSendBuffer);
        #endif
     }
    return;
  }


  if((sequenceState&NKT_PS_PLAYING))
  {

      sNktTrack *pCurTrack=&g_CurrentNktSequence->pTracks[0];

      // update
      pCurTrack->timeElapsedFrac += g_CurrentNktSequence->timeStep;
      TimeAdd = pCurTrack->timeElapsedFrac >> 16;
      pCurTrack->timeElapsedFrac &= 0xffff;

      // timestep forward
      pCurTrack->timeElapsedInt=pCurTrack->timeElapsedInt+TimeAdd;

      bPaused=FALSE;
      bStopped=FALSE;   // we replaying, so we have to reset this flag

      addr=((uint32)pCurTrack->eventBlocksPtr)+ pCurTrack->eventsBlockOffset;
      uint8 count=0;

      // read VLQ delta
      uint8 *pEventPtr=(uint8 *)(addr);
      uint32 currentDelta=readVLQ(pEventPtr,&count);
      pEventPtr+=count;

      // get event block
      nktBlk=(sNktBlock *)(pEventPtr);

      // track end?

      if(nktBlk->msgType&NKT_END || pCurTrack->currentBlockId >= pCurTrack->nbOfBlocks)
      {
             onEndSequence();
             return;
      }

          if( pCurTrack->timeElapsedInt==currentDelta||currentDelta==0)
          {
                pCurTrack->timeElapsedInt -= currentDelta;

              // tempo change ?
              if(nktBlk->msgType&NKT_TEMPO_CHANGE)
              {
                 // set new tempo
                 addr=((uint32)pCurTrack->eventDataPtr)+nktBlk->bufferOffset;
                 uint32 *pMidiDataStartAdr=(uint32 *)(addr);

                 g_CurrentNktSequence->currentTempo.tempo=*pMidiDataStartAdr;
                 pMidiDataStartAdr++;

                 // get precalculated timestep from data buffer
                 g_CurrentNktSequence->timeStep=pMidiDataStartAdr[g_CurrentNktSequence->currentUpdateFreq];

                 //next event
                 pCurTrack->eventsBlockOffset+=count;
                 pCurTrack->eventsBlockOffset+=sizeof(sNktBlock);
                 ++(pCurTrack->currentBlockId);

                 // get next event block
                 addr=((uint32)pCurTrack->eventBlocksPtr)+pCurTrack->eventsBlockOffset;
                 uint8 count=0;

                 // read VLQ delta
                 uint8 *pEventPtr=(uint8 *)(addr);
                 uint32 currentDelta = readVLQ(pEventPtr,&count);
                 pEventPtr+=count;

                 // get event block
                 nktBlk=(sNktBlock *)(pEventPtr);
             }

              uint32 *pMidiDataStartAdr=(uint32 *)(((uint32)pCurTrack->eventDataPtr)+nktBlk->bufferOffset);

     #ifdef IKBD_MIDI_SEND_DIRECT
               amMemCpy(MIDIsendBuffer, pMidiDataStartAdr, nktBlk->blockSize);
               MIDIbytesToSend=nktBlk->blockSize;
       #else
               //send to xbios
               amMidiSendData(nktBlk->blockSize, pMidiDataStartAdr);
       #endif

               //go to next event
               pCurTrack->eventsBlockOffset+=count;
               pCurTrack->eventsBlockOffset+=sizeof(sNktBlock);

               ++(pCurTrack->currentBlockId);

          } // end delta check

  }else{
    // check sequence state if stopped reset position
    // and tempo to default, but only once

   if(bStopped!=TRUE)
   {
      bStopped=TRUE;

      g_CurrentNktSequence->currentTempo.tempo=g_CurrentNktSequence->defaultTempo.tempo;

      // copy / update precalculated tempo data
      g_CurrentNktSequence->currentTempo.tuTable[NKT_U25HZ]=g_CurrentNktSequence->defaultTempo.tuTable[NKT_U25HZ];
      g_CurrentNktSequence->currentTempo.tuTable[NKT_U50HZ]=g_CurrentNktSequence->defaultTempo.tuTable[NKT_U50HZ];
      g_CurrentNktSequence->currentTempo.tuTable[NKT_U100HZ]=g_CurrentNktSequence->defaultTempo.tuTable[NKT_U100HZ];
      g_CurrentNktSequence->currentTempo.tuTable[NKT_U200HZ]=g_CurrentNktSequence->defaultTempo.tuTable[NKT_U200HZ];
   
      TimeAdd = 0;

      // reset tempo to initial valueas taken during start (get them from main sequence?)
      // get precalculated timestep
      g_CurrentNktSequence->timeStep=g_CurrentNktSequence->currentTempo.tuTable[g_CurrentNktSequence->currentUpdateFreq];

      //rewind all tracks to the first event
      for(uint16 i=0;i<g_CurrentNktSequence->nbOfTracks;++i)
      {
          g_CurrentNktSequence->pTracks[i].timeElapsedInt=0UL;
          g_CurrentNktSequence->pTracks[i].timeElapsedFrac=0UL;
          g_CurrentNktSequence->pTracks[i].eventsBlockOffset=0UL;
          g_CurrentNktSequence->pTracks[i].currentBlockId=0UL;
      }

      resetMidiDevice();

    }
   return;
  }

} //end updateStepNkt()



// update step for multitrack replay

void updateStepNktMt(void)
{

 // handle master volume, balance, reverb, mt32 text
 handleMasterSettings();

 if(g_CurrentNktSequence==0) return;

 sequenceState=g_CurrentNktSequence->sequenceState;

 //check sequence state if paused do nothing
 if((sequenceState&NKT_PS_PAUSED)){

     if(bPaused!=TRUE)
     {
        bPaused=TRUE;
        bStopped=FALSE;

        // all notes off but only once
        amAllNotesOff(16);

        #ifdef IKBD_MIDI_SEND_DIRECT
             Supexec(flushMidiSendBuffer);
        #endif
     }
    return;
  }


  if((sequenceState&NKT_PS_PLAYING)){

      TrackEndCount=0;

      for(uint16 i=0;i<g_CurrentNktSequence->nbOfTracks;++i)
      {

      sNktTrack *pCurTrack=&g_CurrentNktSequence->pTracks[i];

      // update
      pCurTrack->timeElapsedFrac += g_CurrentNktSequence->timeStep;
      TimeAdd = pCurTrack->timeElapsedFrac >> 16;
      pCurTrack->timeElapsedFrac &= 0xffff;

      // timestep forward
      pCurTrack->timeElapsedInt=pCurTrack->timeElapsedInt+TimeAdd;

      bPaused=FALSE;
      bStopped=FALSE;   // we replaying, so we have to reset this flag

      addr=((uint32)pCurTrack->eventBlocksPtr)+ pCurTrack->eventsBlockOffset;
      uint8 count=0;

      // read VLQ delta
      uint8 *pEventPtr=(uint8 *)(addr);
      uint32 currentDelta=readVLQ(pEventPtr,&count);
      pEventPtr+=count;

      // get event block
      nktBlk=(sNktBlock *)(pEventPtr);

      // track end?
      if(nktBlk->msgType&NKT_END || pCurTrack->currentBlockId >= pCurTrack->nbOfBlocks){
        // skip update
          if(nktBlk->msgType&NKT_END) ++TrackEndCount;

        continue;
      }

      if( pCurTrack->timeElapsedInt == currentDelta || currentDelta==0 ){
          pCurTrack->timeElapsedInt -= currentDelta;

          // tempo change ?
          if(nktBlk->msgType&NKT_TEMPO_CHANGE){
              // set new tempo
              addr=((uint32)pCurTrack->eventDataPtr)+nktBlk->bufferOffset;
              uint32 *pMidiDataStartAdr=(uint32 *)(addr);

              g_CurrentNktSequence->currentTempo.tempo=*pMidiDataStartAdr;
              pMidiDataStartAdr++;

              // get precalculated timestep from data buffer
              g_CurrentNktSequence->timeStep=pMidiDataStartAdr[g_CurrentNktSequence->currentUpdateFreq];

              //next event
              pCurTrack->eventsBlockOffset+=count;
              pCurTrack->eventsBlockOffset+=sizeof(sNktBlock);
              ++(pCurTrack->currentBlockId);

              // get next event block
              addr=((uint32)pCurTrack->eventBlocksPtr)+pCurTrack->eventsBlockOffset;
              uint8 count=0;

              // read VLQ delta
              uint8 *pEventPtr=(uint8 *)(addr);
              uint32 currentDelta = readVLQ(pEventPtr,&count);
              pEventPtr+=count;

              // get event block
              nktBlk=(sNktBlock *)(pEventPtr);
          }

          uint32 *pMidiDataStartAdr=(uint32 *)(((uint32)pCurTrack->eventDataPtr)+nktBlk->bufferOffset);

#ifdef IKBD_MIDI_SEND_DIRECT
          amMemCpy(MIDIsendBuffer, pMidiDataStartAdr, nktBlk->blockSize);
          MIDIbytesToSend=nktBlk->blockSize;
#else
          //send to xbios
          amMidiSendData(nktBlk->blockSize, pMidiDataStartAdr);
#endif

          //go to next event
          pCurTrack->eventsBlockOffset+=count;
          pCurTrack->eventsBlockOffset+=sizeof(sNktBlock);

          ++(pCurTrack->currentBlockId);

     } // end delta check
   } //end of track processing

     // check end of track
     if(g_CurrentNktSequence->nbOfTracks==TrackEndCount){
         // all tracks reached its end
         onEndSequence();
     }

  }else{
    // check sequence state if stopped reset position
    // and tempo to default, but only once

   if(bStopped!=TRUE)
   {
      bStopped=TRUE;

      g_CurrentNktSequence->currentTempo.tempo=g_CurrentNktSequence->defaultTempo.tempo;

      //copy/update precalculated tempo data
      g_CurrentNktSequence->currentTempo.tuTable[NKT_U25HZ]=g_CurrentNktSequence->defaultTempo.tuTable[NKT_U25HZ];
      g_CurrentNktSequence->currentTempo.tuTable[NKT_U50HZ]=g_CurrentNktSequence->defaultTempo.tuTable[NKT_U50HZ];
      g_CurrentNktSequence->currentTempo.tuTable[NKT_U100HZ]=g_CurrentNktSequence->defaultTempo.tuTable[NKT_U100HZ];
      g_CurrentNktSequence->currentTempo.tuTable[NKT_U200HZ]=g_CurrentNktSequence->defaultTempo.tuTable[NKT_U200HZ];

      TimeAdd = 0;

      TrackEndCount=0;

      // reset tempo to initial valueas taken during start (get them from main sequence?)
      // get precalculated timestep
      g_CurrentNktSequence->timeStep=g_CurrentNktSequence->currentTempo.tuTable[g_CurrentNktSequence->currentUpdateFreq];

      //rewind all tracks to the first event
      for(uint16 i=0;i<g_CurrentNktSequence->nbOfTracks;++i)
      {
          g_CurrentNktSequence->pTracks[i].timeElapsedInt=0;
          g_CurrentNktSequence->pTracks[i].timeElapsedFrac=0;
          g_CurrentNktSequence->pTracks[i].eventsBlockOffset=0;
          g_CurrentNktSequence->pTracks[i].currentBlockId=0;
      }

      resetMidiDevice();

    }
   return;
  }

} //end updateStepNkt()

sNktSeq *loadNktSequence(const uint8 *pFilePath){
    // create header
    sNktSeq *pNewSeq=(sNktSeq *)gUserMemAlloc(sizeof(sNktSeq),PREFER_TT,0);

    if(pNewSeq==0)
    {
     amTrace("Error: Couldn't allocate memory for sequence header."NL,0);
     return NULL;
    }

    amMemSet(pNewSeq,0,sizeof(sNktSeq));
	
    pNewSeq->currentUpdateFreq = NKT_U200HZ;
    pNewSeq->sequenceState |= NKT_PLAY_ONCE;
    pNewSeq->defaultTempo.tempo = DEFAULT_MPQN;
    pNewSeq->currentTempo.tempo = DEFAULT_MPQN;
    pNewSeq->timeDivision = DEFAULT_PPQN;
    pNewSeq->nbOfTracks=1;

    //get nb of blocks from file
#ifdef ENABLE_GEMDOS_IO
    int16 fh=GDOS_INVALID_HANDLE;
#else
    FILE *fp=0;
#endif

    if(pFilePath)
    {
       // create file header
       amTrace("Loading NKT file: %s"NL,pFilePath);

#ifdef ENABLE_GEMDOS_IO
       fh=Fopen(pFilePath,S_READ);
#else
       fp = fopen(pFilePath, "rb"); //read only
#endif

#ifdef ENABLE_GEMDOS_IO
       if(fh<0){
#else
       if(fp==NULL){
#endif
            amTrace("Error: Couldn't open : %s. File doesn't exists."NL,pFilePath);
            gUserMemFree(pNewSeq,0);
            return NULL;
         }
      }else{

        amTrace("Error: empty file path. Exiting..."NL,0);
        gUserMemFree(pNewSeq,0);
        return NULL;
      }

    // read header
    sNktHd tempHd;
    amMemSet(&tempHd,0,sizeof(sNktHd));

#ifdef ENABLE_GEMDOS_IO
    int32 read=Fread(fh,sizeof(sNktHd),&tempHd);

    if(read<0){
          //GEMDOS ERROR TODO, display error for now
          amTrace("[GEMDOS] Error: %s"NL,getGemdosError(read));
    }else{
        if(read<sizeof(sNktHd)){
            amTrace("[GEMDOS] Read error, unexpected EOF. Expected: %d, read: %d"NL,sizeof(sNktHd),read);
        }
    }

#else
      fread(&tempHd,sizeof(sNktHd),1,fp);
#endif

// check header
    if(tempHd.id!=ID_NKT)
    {
        amTrace("Error: File %s isn't valid!/. Exiting..."NL,pFilePath);

        #ifdef ENABLE_GEMDOS_IO
            amTrace("[GEMDOS] Closing file handle : [%d] "NL, fh);
            int16 err=Fclose(fh);

            if(err!=GDOS_OK){
              amTrace("[GEMDOS] Error closing file handle : [%d] %s"NL, fh, getGemdosError(err));
            }

        #else
            fclose(fp); fp=0;
        #endif

     gUserMemFree(pNewSeq,0);
     return NULL;
   }

   //read track data
   sNktTrackInfo *trackData=0;
   trackData=(sNktTrackInfo *)gUserMemAlloc(tempHd.nbOfTracks * sizeof(sNktTrack), PREFER_TT,0);

   if(trackData==NULL){
      amTrace("Error: Couldn't allocate memory for track info."NL,0);
      return NULL;
   }

   amMemSet(trackData,0,sizeof(sNktTrackInfo) * tempHd.nbOfTracks);

#ifdef ENABLE_GEMDOS_IO
    Fseek(sizeof(sNktHd), fh, 0 );
    read=Fread(fh,sizeof(sNktTrackInfo)* tempHd.nbOfTracks,trackData);

    if(read<0){
          // GEMDOS ERROR TODO, display error for now
          amTrace("[GEMDOS] Error: %s"NL,getGemdosError(read));
    }else{
        if(read<(sizeof(sNktTrackInfo)* tempHd.nbOfTracks)){
            amTrace("[GEMDOS] Read error, unexpected EOF. Expected: %d, read: %d"NL,sizeof(sNktTrackInfo)* tempHd.nbOfTracks,read);
        }
    }

#else
      fread(trackData,sizeof(sNktTrackInfo)* tempHd.nbOfTracks,1,fp);
#endif

   amTrace("[NKT header]\nnb of Tracks: %u ", tempHd.nbOfTracks);
   amTrace("td: %u "NL, tempHd.division);

   if(tempHd.version!=NKT_VERSION){
     amTrace("Error: Wrong version of NKT file. Cannot load file."NL,0);
     gUserMemFree(trackData,0);
     return NULL;
   }

   for(uint16 i=0;i<tempHd.nbOfTracks;++i){
     amTrace("[NKT track #%u]\nNb of blocks: %u (%u bytes),\nEvent data buffer size: %u"NL,i,trackData[i].nbOfBlocks, trackData[i].eventsBlockBufSize,trackData[i].eventDataBufSize);
   }

   lzo_voidp pPackedEvents=0;
   lzo_voidp pPackedData=0;
   lzo_uint newEventSize=0;
   lzo_uint newDataSize=0;



   if( trackData[0].nbOfBlocks==0 || trackData[0].eventsBlockBufSize==0 || trackData[0].eventDataBufSize==0 ){

    amTrace(NL " Error: File %s has no data or event blocks!"NL,pFilePath);

    #ifdef ENABLE_GEMDOS_IO
        amTrace("[GEMDOS] Closing file handle : [%d] "NL, fh);
        int16 err=Fclose(fh);

        if(err!=GDOS_OK){
          amTrace("[GEMDOS] Error closing file handle : [%d] %s"NL, fh, getGemdosError(err));
        }

    #else
        fclose(fp); fp=0;
    #endif

    gUserMemFree(trackData,0);
    gUserMemFree(pNewSeq,0);
    return NULL;

   }
   else
   {
        // update info from header and track info data
        pNewSeq->timeDivision = tempHd.division;
        pNewSeq->version = tempHd.version;
        pNewSeq->nbOfTracks = tempHd.nbOfTracks;

        pNewSeq->pTracks=(sNktTrack *)gUserMemAlloc(pNewSeq->nbOfTracks*sizeof(sNktTrack),PREFER_TT,0);

        if(pNewSeq->pTracks==0)
        {
            amTrace("Error: Couldn't allocate memory for track info."NL,0);

            gUserMemFree(pNewSeq,0);
            return NULL;
        }

        // ok
        for(uint16 i=0;i<pNewSeq->nbOfTracks;++i){

            sNktTrack *pTrk=(sNktTrack *)&pNewSeq->pTracks[i];

            pTrk->currentBlockId=0;
            pTrk->eventsBlockOffset=0;

            pTrk->nbOfBlocks = trackData[i].nbOfBlocks;
            pTrk->eventsBlockBufferSize = trackData[i].eventsBlockBufSize;
            pTrk->dataBufferSize = trackData[i].eventDataBufSize;

            amTrace(NL "[Track]: %u "NL, i);
            amTrace("Blocks in sequence: %u "NL, pTrk->nbOfBlocks);
            amTrace("Event data buffer size: %u "NL, pTrk->dataBufferSize);
            amTrace("Events block size: %u "NL, pTrk->eventsBlockBufferSize);


        // packed data check
        if(trackData[i].eventsBlockBufSize!=trackData[i].eventsBlockPackedSize){
            amTrace("[Data packed]"NL,0);

            lzo_voidp pPackedDataSource=0;

            if(lzo_init()!=LZO_E_OK){
              amTrace("Error: Could't initialise LZO library. Cannot depack data. "NL,0);
              return NULL;
            }

            amTrace("[LZO] LZO real-time data compression library (v%s, %s)."NL,
                   lzo_version_string(), lzo_version_date());

                // unpack them to temp buffers

                amTrace("[LZO] Allocating temp events buffer"NL,0);
                uint32 amount = pTrk->eventsBlockBufferSize*3;

                pPackedEvents=(lzo_voidp) gUserMemAlloc(amount, PREFER_TT,0);
                pPackedDataSource=(lzo_voidp) gUserMemAlloc(pTrk->eventsBlockBufferSize,PREFER_TT,0);

                if(pPackedEvents!=NULL&&pPackedDataSource!=NULL){

                    amMemSet(pPackedEvents,0,amount);
                    amMemSet(pPackedDataSource,0,pTrk->eventsBlockBufferSize);

                    // fill buffer with packed data
#ifdef ENABLE_GEMDOS_IO
                    int32 read = Fread(fh, pTrk->eventsBlockBufferSize, pPackedDataSource);

                    if(read!=pTrk->eventsBlockBufferSize){
                        amTrace("[GEMDOS] Error: read %lu, expected: %u "NL,read,pTrk->eventsBlockBufferSize);
                    }else
                        amTrace("[GEMDOS] Read events buffer data %u "NL,read);
#else
                        StaticAssert(0,"Not implemented!");
#endif
                    amTrace("[LZO] Decompressing events block..."NL,0);
                    int decResult=lzo1x_decompress(pPackedDataSource,pTrk->eventsBlockBufferSize,pPackedEvents,&newEventSize,LZO1X_MEM_DECOMPRESS);

                    if( decResult == LZO_E_OK){
                        amTrace("[LZO] Decompressed events buffer %d->%d bytes"NL,pTrk->eventsBlockBufferSize,newEventSize);
                        pTrk->eventsBlockBufferSize=newEventSize;
                    }else{
                        amTrace("[LZO] Error: Events block decompression error: %d at %d"NL,decResult,newEventSize);
                    }

                    //release
                    gUserMemFree(pPackedDataSource,0);
                }

                amTrace("[LZO] Allocating temp data buffer"NL,0);
                amount = pTrk->dataBufferSize*3;

                pPackedData=(lzo_voidp)gUserMemAlloc(amount,PREFER_TT,0);

                pPackedDataSource=(lzo_voidp)gUserMemAlloc(pTrk->dataBufferSize,PREFER_TT,0);


                if(pPackedData!=NULL && pPackedDataSource!=NULL)
                {
                    amMemSet(pPackedData,0,amount);
                    amMemSet(pPackedDataSource,0,pTrk->dataBufferSize);

#ifdef ENABLE_GEMDOS_IO
                    amTrace("[GEMDOS] Read events data buffer"NL,0);
                    int32 read=Fread(fh, pTrk->dataBufferSize, pPackedDataSource);
#else
                        StaticAssert(0,"Not implemented!");
#endif
                    amTrace("[LZO] Decompressing data block..."NL,0);
                    int decResult = lzo1x_decompress(pPackedDataSource,pTrk->dataBufferSize,pPackedData,&newDataSize,LZO1X_MEM_DECOMPRESS);

                    if( decResult == LZO_E_OK){
                        amTrace("[LZO] Decompressed events buffer %d->%d bytes"NL,pTrk->dataBufferSize,newDataSize);
                        pTrk->dataBufferSize=newDataSize;
                    }else{
                        amTrace("[LZO] Error: Data block decompression error: %d at %d"NL, decResult, newDataSize);
                    }

                    //release
                    gUserMemFree(pPackedDataSource,0);
                }

        }

        uint32 lbAllocAdr=0;

        // create linear buffer
        if(createLinearBuffer(&(pTrk->lbEventsBuffer),pTrk->eventsBlockBufferSize+255,PREFER_TT)<0){

            amTrace("Error: loadSequence() Couldn't allocate memory for event block buffer."NL,0);

            #ifdef ENABLE_GEMDOS_IO
                amTrace("[GEMDOS] Closing file handle : [%d] "NL, fh);

                int16 err=Fclose(fh);

                if(err!=GDOS_OK){
                  amTrace("[GEMDOS] Error closing file handle : [%d] %s"NL, fh, getGemdosError(err));
                }

            #else
                fclose(fp); fp=0;
            #endif

            gUserMemFree(pNewSeq,0);
            return NULL;
         }

         // allocate contigous / linear memory for pNewSeq->NbOfBlocks events
         lbAllocAdr=(uint32)linearBufferAlloc(&(pTrk->lbEventsBuffer), (pTrk->eventsBlockBufferSize)+255);
         lbAllocAdr+=255;
         lbAllocAdr&=0xfffffff0;

         pTrk->eventBlocksPtr = (uint8 *)lbAllocAdr;

         if(pTrk->eventBlocksPtr==0){

            amTrace("Error: loadSequence() Linear buffer out of memory."NL,0);

            #ifdef ENABLE_GEMDOS_IO
                amTrace("[GEMDOS] Closing file handle : [%d] "NL, fh);

                int16 err=Fclose(fh);

                if(err!=GDOS_OK){
                  amTrace("[GEMDOS] Error closing file handle : [%d] %s"NL, fh, getGemdosError(err));
                }

            #else
                fclose(fp); fp=0;
            #endif

             gUserMemFree(pNewSeq,0);
             return NULL;
         }

         amTrace("Allocated %u k for event block buffer"NL,pTrk->eventsBlockBufferSize);

         if(createLinearBuffer(&(pTrk->lbDataBuffer),pTrk->dataBufferSize+255,PREFER_TT)<0){

            amTrace("Error: loadSequence() Couldn't allocate memory for temp data buffer. "NL,0);

            #ifdef ENABLE_GEMDOS_IO
                amTrace("[GEMDOS] Closing file handle : [%d] "NL, fh);

                int16 err=Fclose(fh);

                if(err!=GDOS_OK){
                  amTrace("[GEMDOS] Error closing file handle : [%d] %s"NL, fh, getGemdosError(err));
                }

            #else
                fclose(fp); fp=0;
            #endif

             // destroy block buffer
             destroyLinearBuffer(&(pTrk->lbEventsBuffer));
             gUserMemFree(pNewSeq,0);

             return NULL;
         }

       // alloc memory for data buffer from linear allocator
       lbAllocAdr=(uint32)linearBufferAlloc(&(pTrk->lbDataBuffer), pTrk->dataBufferSize+255);
       lbAllocAdr+=255;
       lbAllocAdr&=0xfffffff0;

       pTrk->eventDataPtr = (uint8*)lbAllocAdr;
       amTrace("Allocated %u b for event data buffer"NL,pTrk->dataBufferSize);

       if(pTrk->eventDataPtr==0){

           amTrace("Error: loadSequence() Linear buffer out of memory."NL,0);

           // destroy block buffer
           destroyLinearBuffer(&(pTrk->lbEventsBuffer));
           destroyLinearBuffer(&(pTrk->lbDataBuffer));

           #ifdef ENABLE_GEMDOS_IO
            amTrace("[GEMDOS] Closing file handle : [%d] "NL, fh);

            int16 err=Fclose(fh);

            if(err!=GDOS_OK){
                amTrace("[GEMDOS] Error closing file handle : [%d] %s"NL, fh, getGemdosError(err));
            }

           #else
            fclose(fp); fp=0;
           #endif

           gUserMemFree(pNewSeq,0);
           return NULL;
       }

       // read raw data if file isn't packed
       if(trackData[i].eventsBlockBufSize!=trackData[i].eventsBlockPackedSize){
            // copy depacked data to buffers
            amMemCpy(pTrk->eventBlocksPtr, pPackedEvents, pTrk->eventsBlockBufferSize);
            amMemCpy(pTrk->eventDataPtr, pPackedData, pTrk->dataBufferSize);

            // free temp buffers with packed data
            gUserMemFree(pPackedEvents,0);
            gUserMemFree(pPackedData,0);
       }

       // read raw data if file isn't packed
       if(trackData[i].eventsBlockBufSize == trackData[i].eventsBlockPackedSize){
         // load decompressed data directly from file
         // load event block
            MemSize amount = pTrk->eventsBlockBufferSize;

    #ifdef ENABLE_GEMDOS_IO
             read=Fread(fh,amount,(void *)pTrk->eventBlocksPtr);

             if(read<amount){
                 amTrace("[GEMDOS] error, read bytes: %d, expected read: %d ",read,amount);
                 // todo cleanup
                 return NULL;
             }
    #else
            fread((void *)pTrk->eventBlocksPtr, amount, 1, fp);
    #endif

            amount = pTrk->dataBufferSize;

            // load data block
    #ifdef ENABLE_GEMDOS_IO
            read=Fread(fh,amount,(void *)pTrk->eventDataPtr);

            if(read<amount){
                amTrace("[GEMDOS] error, read bytes: %d, expected read: %d ",read,amount);
                // todo cleanup
                return NULL;
            }
     #else
            fread((void *)pTrk->eventDataPtr,amount,1,fp);
    #endif
     }

     } // end track for loop

}//end track process

#ifdef LOAD_TEST
    amTrace("[LOAD TEST]"NL,0);
    uint32 blockNb=0;
    uint8 count=0;

   for(uint16 i=0;i<pNewSeq->nbOfTracks;++i){
    amTrace("[Track #%u]"NL,i);

    uint32 blockNb=0;
    uint8 count=0;

    while(blockNb<pNewSeq->pTracks[i].nbOfBlocks){

      uint32 addr=((uint32)pNewSeq->pTracks[i].eventBlocksPtr) + pNewSeq->pTracks[i].eventsBlockOffset;

      uint8 *pEventPtr=(uint8 *)(addr);
      uint32 d=readVLQ(pEventPtr,&count);

      pEventPtr+=count;

      sNktBlock *eBlk=(sNktBlock *)(pEventPtr);

      amTrace("delta [%u] type:[%hu] size:[%hu] bytes offset: [%u] "NL,d, eBlk->msgType, eBlk->blockSize,eBlk->bufferOffset);

      if(eBlk->blockSize>0){
          amTrace("[DATA] ",0);

          uint8 *data = (uint8 *)((uint32)(pNewSeq->pTracks[i].eventDataPtr)+eBlk->bufferOffset);

          for(int j=0;j<eBlk->blockSize;++j){
           amTrace("0x%02x ",data[j]);
          }

          amTrace(" [/DATA]"NL,0);
      }

      pNewSeq->pTracks[i].eventsBlockOffset+=count;
      pNewSeq->pTracks[i].eventsBlockOffset+=sizeof(sNktBlock);

      ++blockNb;
    }
   }
#endif



// close file
#ifdef ENABLE_GEMDOS_IO
    amTrace("[GEMDOS] Closing file handle : [%d] "NL, fh);
    int16 err=Fclose(fh);

    if(err!=GDOS_OK){
        amTrace("[GEMDOS] Error closing file handle : [%d] %s"NL, fh, getGemdosError(err));
    }

#else
    fclose(fp);fp=0;
#endif

 return pNewSeq;
}

void destroyNktSequence(sNktSeq *pSeq)
{
  AssertMsg(pSeq!=0,"Destroy sequence called on NULL sequence.");

  if(pSeq->pTracks)
  {

   for(uint16 i=0; i<pSeq->nbOfTracks; ++i)
   {
        if(pSeq->pTracks[i].nbOfBlocks!=0)
        {
           // release linear buffer
            linearBufferFree(&(pSeq->pTracks[i].lbEventsBuffer));
            linearBufferFree(&(pSeq->pTracks[i].lbDataBuffer));
        }
   };

   gUserMemFree(pSeq->pTracks,0);
   pSeq->pTracks=0;
 }

 amMemSet(pSeq,0,sizeof(sNktSeq));
 gUserMemFree(pSeq,0);
 return;
}


////////////////////////////////////////////////// replay control

Bool isNktSequencePlaying(void)
{
  AssertMsg(g_CurrentNktSequence!=0,"Destroy sequence called on NULL sequence.");

  const uint16 state = g_CurrentNktSequence->sequenceState;
 
  if((state&NKT_PS_PLAYING)&&(!(state&NKT_PS_PAUSED))) 
    return TRUE;
 
 return FALSE;
}


void stopNktSequence(void)
{
  AssertMsg(g_CurrentNktSequence!=0,"stop sequence called on NULL sequence.");

  const uint16 state=g_CurrentNktSequence->sequenceState;

  if((state&NKT_PS_PLAYING)||(state&NKT_PS_PAUSED))
  {
    g_CurrentNktSequence->sequenceState&=(~(NKT_PS_PLAYING|NKT_PS_PAUSED));

    setMT32Message("Stopped...");
  }

  resetMidiDevice();
}

void pauseNktSequence()
{
  AssertMsg(g_CurrentNktSequence!=0,"Pause sequence called on NULL sequence.");

  const uint16 state = g_CurrentNktSequence->sequenceState;

  if((state&NKT_PS_PLAYING)&&(!(state&NKT_PS_PAUSED)))
  {
    g_CurrentNktSequence->sequenceState&=(~NKT_PS_PLAYING);
    g_CurrentNktSequence->sequenceState|=NKT_PS_PAUSED;

    setMT32Message("Paused...");
  } 
  else if(!(state&NKT_PS_PLAYING)&&(state&NKT_PS_PAUSED) )
  {
    g_CurrentNktSequence->sequenceState&=(~NKT_PS_PAUSED); //unpause
    g_CurrentNktSequence->sequenceState|=NKT_PS_PLAYING;  //set playing state

    setMT32Message("Playing...");
  }
 } //pauseSequence

// play sequence
void playNktSequence(void)
{
  AssertMsg(g_CurrentNktSequence!=0,"Play sequence called on NULL sequence.");

  const uint16 state=g_CurrentNktSequence->sequenceState;

    if(!(state&NKT_PS_PLAYING))
    {
      g_CurrentNktSequence->sequenceState&=(~(NKT_PS_PAUSED));
      g_CurrentNktSequence->sequenceState|=NKT_PS_PLAYING;

      amTrace("Play sequence\t",0);

      if(g_CurrentNktSequence->sequenceState&NKT_PLAY_ONCE)
      {
        amTrace("[ ONCE ]"NL,0);
      }
      else
      {
        amTrace("[ LOOP ]"NL,0);
      }

      }
}

void switchNktReplayMode(void)
{
  AssertMsg(g_CurrentNktSequence!=0,"Switvh replay modecalled on NULL sequence.");

  if(g_CurrentNktSequence->sequenceState&NKT_PLAY_ONCE)
  {
    g_CurrentNktSequence->sequenceState&=(~NKT_PLAY_ONCE);
    amTrace("Set replay mode: [ LOOP ]"NL,0);
  }
  else
  {
    g_CurrentNktSequence->sequenceState|=NKT_PLAY_ONCE;
    amTrace("Set replay mode: [ ONCE ]"NL,0);
  }
}

AM_EXTERN void installMidiResetHandler(void);

void NktInit(const eMidiDeviceType devType, const uint8 channel)
{

    amSetDefaultUserMemoryCallbacks();
    
    initLogger("nkt.log");

    Supexec(installMidiResetHandler);

    // now depending on the connected device type and chosen operation mode
    // set appropriate channel
    // prepare device for receiving messages

    setupMidiDevice(devType,channel);
    setMT32Message("AMIDILIB init...");
}


void NktDeinit()
{
  setMT32Message("Bye ! ;-)");
  deinitLogger();
}

#ifdef DEBUG_BUILD

// debug stuff
static const uint8 *getSequenceStateStr(const uint16 state)
{

 if( !(state&NKT_PS_PLAYING) && (state&NKT_PS_PAUSED) ){
    return "Paused";
 }else if(state&NKT_PS_PLAYING && (!(state&NKT_PS_PAUSED))){
    return "Playing";
 }else if(!(state&NKT_PS_PLAYING)){
    return "Stopped...";
 }
}


void printNktSequenceState(void)
{

if(g_CurrentNktSequence)
{
    amTrace("Td/PPQN: %u"NL,g_CurrentNktSequence->timeDivision);
    amTrace("Time step: %u"NL,g_CurrentNktSequence->timeStep);

    for(uint16 i=0;i<g_CurrentNktSequence->nbOfTracks;++i)
    {
        amTrace("[%d] Time elapsedFrac: %u\t",i,g_CurrentNktSequence->pTracks[i].timeElapsedFrac);
        amTrace("\tTime elapsed: %u"NL,g_CurrentNktSequence->pTracks[i].timeElapsedInt);
    }

    amTrace("\tDefault Tempo: %u"NL,g_CurrentNktSequence->defaultTempo.tempo);
    amTrace("\tLast Tempo: %u"NL,g_CurrentNktSequence->currentTempo.tempo);

    amTrace("\tSequence state: %s"NL,getSequenceStateStr(g_CurrentNktSequence->sequenceState));
  }

 printMidiSendBufferState();

}

static const uint8* _arNktEventName[NKT_MAX_EVENT]={
    "NKT_MIDIDATA",
    "NKT_TEMPO_CHANGE",
    "NKT_JUMP",
    "NKT_TRIGGER",
    "NKT_END"
};

const uint8 *getEventTypeName(uint16 type)
{
    switch(type)
    {
        case NKT_MIDIDATA: return _arNktEventName[0]; break;
        case NKT_TEMPO_CHANGE: return _arNktEventName[1]; break;
        case NKT_JUMP: return _arNktEventName[2]; break;
        case NKT_TRIGGER: return _arNktEventName[3]; break;
        case NKT_END: return _arNktEventName[4]; break;
        default: return 0;
    }
}
#endif


int32 saveEventDataBlocks(int16 fh, sNktSeq *pSeq)
{
        // save data blocks
        for(uint16 i=0;i<pSeq->nbOfTracks;++i){
            int32 written=0;

            // save event block
            amTrace("[MID2NKT] Saving event block.[%d bytes] for track [%d] "NL,pSeq->pTracks[i].eventsBlockBufferSize, i);

#ifdef ENABLE_GEMDOS_IO
            written = Fwrite(fh,pSeq->pTracks[i].eventsBlockBufferSize,(void *)pSeq->pTracks[i].eventBlocksPtr);

            if(written<pSeq->pTracks[i].eventsBlockBufferSize){
               amTrace("[GEMDOS]Fatal error: Events block write error, written: %d , expected %d bytes"NL, written, pSeq->pTracks[i].eventsBlockBufferSize);
               amTrace("[GEMDOS] Error: %s"NL, getGemdosError((int16)written));

               return -1;
            }else{
                amTrace("[GEMDOS] written: %d bytes"NL, written);
            }
#else
            StaticAssert(0,"Not implemented!");
#endif            
            // save data block
            amTrace("[MID2NKT] Saving data block.[%d bytes] for track [%d]"NL,pSeq->pTracks[i].dataBufferSize, i);

#ifdef ENABLE_GEMDOS_IO
            written=Fwrite(fh,pSeq->pTracks[i].dataBufferSize,(void *)pSeq->pTracks[i].eventDataPtr);

            if(written<pSeq->pTracks[i].dataBufferSize)
            {
               amTrace("[GEMDOS]Fatal error: Event data block write error, written: %d , expected %d bytes"NL, written, pSeq->pTracks[i].dataBufferSize);
               amTrace("[GEMDOS] Error: %s"NL, getGemdosError((int16)written));

               return -1;
            }else{
                amTrace("[GEMDOS] written: %d bytes"NL, written);
            }
#else
            StaticAssert(0,"Not implemented!");
#endif            
        }

    return 0;
}



int16 saveNktSequence(sNktSeq *pSeq, const uint8 *filepath, Bool bCompress)
{

if(filepath==0||strlen(filepath)==0) {
    amTrace("[MID2NKT] Fatal error, path is empty."NL,0);
    return AM_ERR;
}

// create header
sNktHd nktHd;
sNktTrackInfo *pTrackInfo=0;

if(pSeq->nbOfTracks==0)
{
    amTrace("[MID2NKT] Fatal error, no tracks in sequence!"NL,0);
    return AM_ERR;
}

pTrackInfo=(sNktTrackInfo *)gUserMemAlloc( (sizeof(sNktTrackInfo) * pSeq->nbOfTracks), PREFER_TT, 0);

if(pTrackInfo==0) 
{
    amTrace("[MID2NKT] Fatal error, no memory for track info!"NL,0);
    return AM_ERR;
}

// set header
amTrace("[MID2NKT] Init header..."NL,0);

setNktHeader(&nktHd, pSeq);
setNktTrackInfo(pTrackInfo,pSeq);

#ifdef ENABLE_GEMDOS_IO
 int16 fh=GDOS_INVALID_HANDLE;

 amTrace("[GEMDOS] Save sequence to %s, compress: %d"NL,filepath, bCompress);

 // file create
 fh = Fcreate(filepath,0);

 if(fh<0){
     amTrace("[GEMDOS] Error: %s"NL, getGemdosError(fh));
     return AM_ERR;
 }

     if(bCompress==TRUE)
     {
          amTrace("[MID2NKT] LZO compression ..."NL,0);

         if(lzo_init()!=LZO_E_OK){
           amTrace("Error: Could't initialise LZO library. "NL,0);
           return AM_ERR;
         }

          amTrace("[LZO] \nLZO real-time data compression library (v%s, %s)."NL,
                 lzo_version_string(), lzo_version_date());

          // allocate work buffers
          uint32 workMemSize=(LZO1X_1_MEM_COMPRESS + (sizeof(lzo_align_t)-1)/sizeof(lzo_align_t))*sizeof(lzo_align_t);

          amTrace("[LZO] Allocating work buffer: %d bytes"NL,workMemSize);

          lzo_voidp workMem=(lzo_voidp)gUserMemAlloc(workMemSize,PREFER_TT,0); // lzo work buffer

          if(workMem!=0){

              amMemSet(workMem,0,workMemSize);

              amTrace("[LZO] Compressing events block."NL,0);
              MemSize tempBufSize=(pSeq->pTracks[0].eventsBlockBufferSize+pSeq->pTracks[0].eventsBlockBufferSize/16+64+3);
              lzo_bytep tempBuffer=(lzo_bytep)gUserMemAlloc(tempBufSize,PREFER_TT,0);

              if(tempBuffer==NULL){
                  amTrace("[LZO] Error, no memory for events block output buffer."NL,0);

                  // free work mem
                  gUserMemFree(workMem,0);
                  return AM_ERR;
              }else{
                  amMemSet(tempBuffer,0,tempBufSize);
              }

              // compress
              lzo_uint nbBytesPacked=0;
              if(lzo1x_1_compress(pSeq->pTracks[0].eventBlocksPtr,pSeq->pTracks[0].eventsBlockBufferSize,tempBuffer,&nbBytesPacked,workMem)==LZO_E_OK){
                    amTrace("[LZO] Event data compressed %u->%u bytes."NL,pSeq->pTracks[0].eventsBlockBufferSize, nbBytesPacked);

                    /* check for an incompressible block */
                    if (nbBytesPacked >= pSeq->pTracks[0].eventsBlockBufferSize)
                    {
                        amTrace("[LZO] Error: Event block contains incompressible data."NL,0);
                        return AM_ERR;
                    }

                    // copy output buffer with packed data
                    pSeq->pTracks[0].eventsBlockBufferSize = nbBytesPacked;
                    pTrackInfo[0].eventsBlockPackedSize = nbBytesPacked;
                    amMemCpy(pSeq->pTracks[0].eventBlocksPtr,tempBuffer,nbBytesPacked);

              }else{
                 amTrace("[LZO] Internal error: Compression failed."NL,0);
              }

              gUserMemFree(tempBuffer,0);

              amTrace("[LZO] Compressing data block."NL,0);
              tempBufSize=pSeq->pTracks[0].dataBufferSize+pSeq->pTracks[0].dataBufferSize/16+64+3;
              tempBuffer=(lzo_bytep)gUserMemAlloc(tempBufSize,PREFER_TT,0);

              if(tempBuffer==NULL){
                  amTrace("[LZO] Error, no memory for data block output buffer."NL,0);

                  // free work mem
                  gUserMemFree(workMem,0);
                  return AM_ERR;
              }else{
                  amMemSet(tempBuffer,0,tempBufSize);
              }

              // compress data block
              nbBytesPacked=0;
              if(lzo1x_1_compress(pSeq->pTracks[0].eventDataPtr,pSeq->pTracks[0].dataBufferSize,tempBuffer,&nbBytesPacked,workMem)==LZO_E_OK){
                    amTrace("[LZO] Data block compressed %u->%u bytes."NL,pSeq->pTracks[0].dataBufferSize, nbBytesPacked);

                    /* check for an incompressible block */
                      if (nbBytesPacked >= pSeq->pTracks[0].dataBufferSize)
                      {
                        amTrace("[LZO] Error: Data block contains incompressible data."NL,0);
                        return AM_ERR;
                      }

                    // copy output buffer with packed data
                    // TODO: shrink existing buffer somehow

                    pSeq->pTracks[0].dataBufferSize=nbBytesPacked;
                    pTrackInfo[0].eventDataBlockPackedSize = nbBytesPacked;

                    amMemCpy(pSeq->pTracks[0].eventDataPtr,tempBuffer,nbBytesPacked);

              }else{
                  amTrace("[LZO] Internal error: Compression failed."NL,0);
              }

              //copy output buffer with packed data to
              gUserMemFree(tempBuffer,0);

              // free work mem
              gUserMemFree(workMem,0);

          }else{
              amTrace("[LZO] Error couldn't allocate compression work memory."NL,0);
              return AM_ERR;
          }

          // save header
          int32 written=0;

          // save header
          amTrace("[MID2NKT] Saving header... "NL,0);

#ifdef ENABLE_GEMDOS_IO
          written=Fwrite(fh, sizeof(sNktHd), &nktHd);

          if(written<sizeof(sNktHd)){
             amTrace("[GEMDOS]Fatal error: Header write error, written: %d, expected %d"NL, written, sizeof(sNktHd));
             amTrace("[GEMDOS] Error: %s"NL, getGemdosError((int16)written));
             return AM_ERR;
          }else{
              amTrace("[GEMDOS] written: %d bytes"NL, written);
          }

          // save track data
          written=Fwrite(fh, sizeof(sNktTrackInfo) * pSeq->nbOfTracks, pTrackInfo);

          if(written<sizeof(sizeof(sNktTrackInfo) * pSeq->nbOfTracks)){
             amTrace("[GEMDOS]Fatal error: Track write error, written: %d, expected %d"NL, written, sizeof(sNktTrackInfo) * pSeq->nbOfTracks);
             amTrace("[GEMDOS] Error: %s"NL, getGemdosError((int16)written));
             return AM_ERR;
          }else{
              amTrace("[GEMDOS] written: %d bytes"NL, written);
          }
#else
          StaticAssert(0,"Not implemented!");
#endif

          // write data / event blocks
          if(saveEventDataBlocks(fh,pSeq)<0){
              return AM_ERR;
          }

     }else{

         amTrace("[GEMDOS] Created file handle: %d"NL,fh);

         int32 written=0;

         // save header
         amTrace("[MID2NKT] Saving header... "NL,0);

#ifdef ENABLE_GEMDOS_IO
         written=Fwrite(fh, sizeof(sNktHd), &nktHd);

         if(written<sizeof(sNktHd)){
            amTrace("[GEMDOS]Fatal error: Header write error, written: %d, expected %d"NL, written, sizeof(sNktHd));
            amTrace("[GEMDOS] Error: %s"NL, getGemdosError((int16)written));
            return AM_ERR;
         }else{
             amTrace("[GEMDOS] written: %d bytes"NL, written);
         }

         // save track data
         written=Fwrite(fh, sizeof(sNktTrackInfo) * pSeq->nbOfTracks, pTrackInfo);

         if(written<sizeof(sizeof(sNktTrackInfo) * pSeq->nbOfTracks)){
            amTrace("[GEMDOS]Fatal error: Track write error, written: %d, expected %d"NL, written, sizeof(sNktTrackInfo) * pSeq->nbOfTracks);
            amTrace("[GEMDOS] Error: %s"NL, getGemdosError((int16)written));
            return AM_ERR;
         }else{
             amTrace("[GEMDOS] written: %d bytes"NL, written);
         }
#else
         StaticAssert(0,"Not implemented!");
#endif         
         // write data / event blocks
         if(saveEventDataBlocks(fh,pSeq)<0){
             return AM_ERR;
         }

     }

     // close file
     int16 err = Fclose(fh);

     if(err!=GDOS_OK){
       amTrace("[GEMDOS] Error closing file handle : [%d] %s"NL, fh, getGemdosError(err));
       return AM_ERR;
     }else{
         amTrace("[GEMDOS] Closed file handle : [%d] "NL, fh);
     }


#else

FILE *file;
amTrace("Save sequence to %s, compress: %d"NL,filepath,bCompress);
StaticAssert(0,"Not implemented!");

 // file create
 // save header
 // save track data

 // for each track:
 //// save event block
 //// save data block

 // close file

#endif

 return AM_OK;
}


void setNktHeader(sNktHd* header, const sNktSeq *pNktSeq)
{
  AssertMsg(header!=0,"Header is NULL!");

  // clear header
  amMemSet(header, 0L, sizeof(sNktHd));
  header->id=ID_NKT;
  header->nbOfTracks=pNktSeq->nbOfTracks;
  header->division = pNktSeq->timeDivision;
  header->version = NKT_VERSION;
}

void setNktTrackInfo(sNktTrackInfo* trackInfo, const sNktSeq *pNktSeq)
{
    AssertMsg(trackInfo!=0,"TrackInfo is NULL");

    for(uint16 i=0;i<pNktSeq->nbOfTracks;++i)
    {
      trackInfo[i].nbOfBlocks = pNktSeq->pTracks[i].nbOfBlocks;
      trackInfo[i].eventDataBlockPackedSize = trackInfo[i].eventDataBufSize = pNktSeq->pTracks[i].dataBufferSize;
      trackInfo[i].eventsBlockPackedSize = trackInfo[i].eventsBlockBufSize = pNktSeq->pTracks[i].eventsBlockBufferSize;
      trackInfo[i].nbOfBlocks = pNktSeq->pTracks[i].nbOfBlocks;

      amTrace("Set track [%d]: event data buffer: %d events block buffer: %d"NL, i, trackInfo[i].eventDataBufSize,trackInfo[i].eventsBlockBufSize);
    }

}
