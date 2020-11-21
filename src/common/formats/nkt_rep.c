
/**  Copyright 2007-2020 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#include "nkt.h"
#include "nkt_util.h"

#include "memory.h"
#include "amlog.h"

#include "timing/mfp.h"
#include "timing/miditim.h"

#include "midi.h"

#include "midi_cmd.h"
#include "rol_ptch.h"

#include "minilzo.h" //lzo pack / depack

#include <stdio.h>

#ifdef ENABLE_GEMDOS_IO
#include "fmio.h"
#include <mint/ostruct.h>
#include <mint/osbind.h>
#endif

// helper function for determining amount of memory we need for data / events buffers
extern uint32 collectMidiTrackInfo(void *pMidiData, uint16 trackNb, sMidiTrackInfo_t *pBufInfo, Bool *bEOT);
extern uint16 isMultitrackReplay;

void setNktHeader(sNktHd* header, const sNktSeq *pNktSeq);
void setNktTrackInfo(sNktTrackInfo* header, const sNktSeq *pNktSeq);

static sNktSeq *g_CurrentNktSequence=0;

void getCurrentSequence(sNktSeq **pSeq){
  *pSeq=g_CurrentNktSequence;
}

static void resetMidiDevice(void){

    amAllNotesOff(16);

    // reset all controllers
    for(uint8 i=0;i<16;++i){
      reset_all_controllers(i);
      omni_off(i);
    }


    #ifdef IKBD_MIDI_SEND_DIRECT
      Supexec(flushMidiSendBuffer);
    #endif

}

// this is called when sequence ends
static void onEndSequence(void){

if(g_CurrentNktSequence){

  if(g_CurrentNktSequence->sequenceState&NKT_PLAY_ONCE){
    // set state to stopped
    // reset song position on all tracks
    g_CurrentNktSequence->sequenceState&=(uint16)(~(NKT_PS_PLAYING|NKT_PS_PAUSED));

  }else{
    // loop
    g_CurrentNktSequence->sequenceState&=(uint16)(~NKT_PS_PAUSED);
    g_CurrentNktSequence->sequenceState|=(uint16)NKT_PS_PLAYING;
  }

  // reset all tracks state

  for(int i=0;i<g_CurrentNktSequence->nbOfTracks;++i){
    g_CurrentNktSequence->pTracks[i].timeElapsedInt=0L;
    g_CurrentNktSequence->pTracks[i].timeElapsedFrac=0L;
    g_CurrentNktSequence->pTracks[i].currentBlockId=0L;
    g_CurrentNktSequence->pTracks[i].eventsBlockOffset=0L;
  }

  g_CurrentNktSequence->currentTempo.tempo=g_CurrentNktSequence->defaultTempo.tempo;
  g_CurrentNktSequence->timeStep=g_CurrentNktSequence->defaultTempo.tuTable[g_CurrentNktSequence->currentUpdateFreq];

  resetMidiDevice();

 }

}

// init sequence
void initSequence(sNktSeq *pSeq, uint16 initialState, bool bInstallUpdate){
 g_CurrentNktSequence=0;

if(pSeq!=0){
    g_CurrentNktSequence=pSeq;

    pSeq->currentUpdateFreq=NKT_U200HZ;
    pSeq->currentTempo.tempo = DEFAULT_MPQN;

    uint32 td=pSeq->timeDivision;
    uint32 bpm = DEFAULT_BPM;
    uint32 tempPPU = bpm * td;

    // precalc tempo table
    amTrace("Precalculating update step for Td: %d, Bpm: %d\n",td,bpm);
     // precalculate values for different update steps

     for(int i=0;i<NKT_UMAX;++i){

          switch(i){
              case NKT_U25HZ:{
                  if(tempPPU<65536){
                      pSeq->defaultTempo.tuTable[i]=pSeq->currentTempo.tuTable[i]=((tempPPU*65536)/60)/25;
                  }else{
                        pSeq->defaultTempo.tuTable[i]=pSeq->currentTempo.tuTable[i]=((tempPPU/60)*65536)/25;
                  }
                  amTrace("Update 25hz: %ld  [0x%x]\n",pSeq->currentTempo.tuTable[i],pSeq->currentTempo.tuTable[i]);
              } break;
              case NKT_U50HZ:{
                  if(tempPPU<65536){
                        pSeq->defaultTempo.tuTable[i]=pSeq->currentTempo.tuTable[i]=((tempPPU*65536)/60)/50;
                  }else{
                        pSeq->defaultTempo.tuTable[i]=pSeq->currentTempo.tuTable[i]=((tempPPU/60)*65536)/50;
                  }
                   amTrace("Update 50hz: %ld [0x%x]\n",pSeq->currentTempo.tuTable[i],pSeq->currentTempo.tuTable[i]);
              } break;
              case NKT_U100HZ:{
                  if(tempPPU<65536){
                       pSeq->defaultTempo.tuTable[i]=pSeq->currentTempo.tuTable[i]=((tempPPU*65536)/60)/100;
                  }else{
                       pSeq->defaultTempo.tuTable[i]=pSeq->currentTempo.tuTable[i]=((tempPPU/60)*65536)/100;
                  }
                   amTrace("Update 100hz: %ld [0x%x]\n",pSeq->currentTempo.tuTable[i],pSeq->currentTempo.tuTable[i]);
              } break;
              case NKT_U200HZ:{
                  if(tempPPU<65536){
                       pSeq->defaultTempo.tuTable[i]=pSeq->currentTempo.tuTable[i]=((tempPPU*65536)/60)/200;
                  }else{
                       pSeq->defaultTempo.tuTable[i]=pSeq->currentTempo.tuTable[i]=((tempPPU/60)*65536)/200;
                  }
                   amTrace("Update 200hz: %ld [0x%x]\n",pSeq->currentTempo.tuTable[i],pSeq->currentTempo.tuTable[i]);
              } break;
              default:{
                  amTrace((const uint8*)"[Error] Invalid timer update value %d\n", i);
              } break;
          };

     } //end for



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
    if(bInstallUpdate!=FALSE){

        if(pSeq->nbOfTracks==1){
             amTrace((const uint8*)"Setting single track replay\n");
            isMultitrackReplay=0;
            Supexec(NktInstallReplayRout);
        }else{
            amTrace((const uint8*)"Setting multitrack replay \n");
            isMultitrackReplay=1;
            Supexec(NktInstallReplayRout);
        }

    }else{
            if(pSeq->nbOfTracks==1){
                 amTrace((const uint8*)"Setting single track replay\n");
                isMultitrackReplay=0;
                Supexec(NktInstallReplayRoutNoTimers);
            }else{
                amTrace((const uint8*)"Setting multitrack replay \n");
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
void initSequenceManual(sNktSeq *pSeq, const uint16 state){
 g_CurrentNktSequence=0;

 if(pSeq!=0){
  uint8 mode=0,data=0;
  g_CurrentNktSequence=pSeq;

  pSeq->currentTempo.tempo = pSeq->defaultTempo.tempo;

  for(int i=0;i<pSeq->nbOfTracks;++i){
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

  amTrace("%dhz update interval, Time step: %d\r\n",SEQUENCER_UPDATE_HZ,pSeq->timeStep);
  amTrace("calculated mode: %d, data: %d\n",mode,data);

  //printNktSequenceState();

 } //endif
 return;
}
#endif

volatile static Bool bStopped=FALSE;
volatile static Bool bPaused=FALSE;
volatile static uint32 TimeAdd=0;
volatile static uint32 addr;
volatile static sNktBlock_t *nktBlk=0;
volatile static uint16 TrackEndCount=0;

volatile uint8 requestedMasterVolume;
volatile uint8 requestedMasterBalance;
volatile static uint16 sequenceState;

volatile sMidiModuleSettings _moduleSettings;
uint8 _mt32TextMsg[20];

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


__attribute__((always_inline)) static inline void handleMasterSettings(void) {

    // handle volume/balance/reverb change
   if(_moduleSettings.masterVolume!=requestedMasterVolume){

       if(MT32_MODEL_ID==_moduleSettings.modelID){

            //handle mt32 volume
            if(requestedMasterVolume<=MIDI_MASTER_VOL_MAX_MT32){

                arSetMasterVolumeMT32.data[IDX_VENDOR]=_moduleSettings.vendorID;
                arSetMasterVolumeMT32.data[IDX_DEVICE_ID]=_moduleSettings.deviceID;
                arSetMasterVolumeMT32.data[IDX_MODEL_ID]=_moduleSettings.modelID;

                arSetMasterVolumeMT32.data[IDX_CMD_ID]=0x12;                          // sending
                arSetMasterVolumeMT32.data[IDX_MASTER_VOL]=requestedMasterVolume;
                arSetMasterVolumeMT32.data[9]=amCalcRolandChecksum(&arSetMasterVolumeMT32.data[5],&arSetMasterVolumeMT32.data[8]);

                sendSysEX(&arSetMasterVolumeMT32);

                #ifdef IKBD_MIDI_SEND_DIRECT
                    Supexec(flushMidiSendBuffer);
                #endif

                _moduleSettings.masterVolume=requestedMasterVolume;
            }

            if(_mt32TextMsg[0]!=0){

                amMemSet((void *)&(arSetTextMT32.data[8]),0,sizeof(uint8)*20);

                arSetTextMT32.data[IDX_VENDOR]=_moduleSettings.vendorID;
                arSetTextMT32.data[IDX_DEVICE_ID]=_moduleSettings.deviceID;
                arSetTextMT32.data[IDX_MODEL_ID]=_moduleSettings.modelID;
                arSetTextMT32.data[IDX_CMD_ID]=0x12;

                amMemCpy(&arSetTextMT32.data[8],&_mt32TextMsg[0],sizeof(uint8)*20);
                arSetTextMT32.data[28]=amCalcRolandChecksum(&arSetTextMT32.data[5],&arSetTextMT32.data[27]);

                // update text
                sendSysEX(&arSetTextMT32);

                #ifdef IKBD_MIDI_SEND_DIRECT
                    Supexec(flushMidiSendBuffer);
                #endif

                // reset text
                amMemSet(&_mt32TextMsg[0],0,sizeof(uint8)*20);
            }


            // todo reverb change request

            // update text

       }else{
        // General Midi GS, todo move it to configurable callback
            // send new master vol
            if(requestedMasterVolume<=MIDI_MASTER_VOL_MAX_GM){

                arSetMasterVolumeGM.data[IDX_VENDOR]=_moduleSettings.vendorID;
                arSetMasterVolumeGM.data[IDX_DEVICE_ID]=_moduleSettings.deviceID;
                arSetMasterVolumeGM.data[IDX_MODEL_ID]=_moduleSettings.modelID;

                arSetMasterVolumeGM.data[IDX_CMD_ID]=0x12;                         // sending
                arSetMasterVolumeGM.data[IDX_MASTER_VOL]=requestedMasterVolume;
                arSetMasterVolumeGM.data[9]=amCalcRolandChecksum(&arSetMasterVolumeGM.data[5],&arSetMasterVolumeGM.data[8]);

                sendSysEX(&arSetMasterVolumeGM);

                #ifdef IKBD_MIDI_SEND_DIRECT
                    Supexec(flushMidiSendBuffer);
                #endif

                _moduleSettings.masterVolume=requestedMasterVolume;

            }
         }

         if(_moduleSettings.masterBalance!=requestedMasterBalance){

             // send new balance
            arSetMasterBalanceGM.data[IDX_VENDOR]=_moduleSettings.vendorID;
            arSetMasterBalanceGM.data[IDX_DEVICE_ID]=_moduleSettings.deviceID;
            arSetMasterBalanceGM.data[IDX_MODEL_ID]=_moduleSettings.modelID;
            arSetMasterBalanceGM.data[IDX_CMD_ID]=0x12;                                // sending
            arSetMasterBalanceGM.data[IDX_MASTER_PAN]=requestedMasterBalance;
            arSetMasterBalanceGM.data[9]=amCalcRolandChecksum(&arSetMasterBalanceGM.data[5],&arSetMasterBalanceGM.data[8]);

            sendSysEX(&arSetMasterBalanceGM);

            #ifdef IKBD_MIDI_SEND_DIRECT
             Supexec(flushMidiSendBuffer);
            #endif

            _moduleSettings.masterBalance=requestedMasterBalance;
         }
    }

}

//update step for single track replay

void updateStepNkt(void){

 // handle master volume, balance, reverb, mt32 text
 handleMasterSettings();

 if(g_CurrentNktSequence==0) return;

 sequenceState=g_CurrentNktSequence->sequenceState;

 //check sequence state if paused do nothing
 if((sequenceState&NKT_PS_PAUSED)){

     if(bPaused==FALSE){
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
      nktBlk=(sNktBlock_t *)(pEventPtr);

      // track end?

          if(nktBlk->msgType&NKT_END || pCurTrack->currentBlockId >= pCurTrack->nbOfBlocks){
             onEndSequence();
             return;
         }

          if( pCurTrack->timeElapsedInt==currentDelta||currentDelta==0){
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
                 pCurTrack->eventsBlockOffset+=sizeof(sNktBlock_t);
                 ++(pCurTrack->currentBlockId);

                 // get next event block
                 addr=((uint32)pCurTrack->eventBlocksPtr)+pCurTrack->eventsBlockOffset;
                 uint8 count=0;

                 // read VLQ delta
                 uint8 *pEventPtr=(uint8 *)(addr);
                 uint32 currentDelta = readVLQ(pEventPtr,&count);
                 pEventPtr+=count;

                 // get event block
                 nktBlk=(sNktBlock_t *)(pEventPtr);
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
               pCurTrack->eventsBlockOffset+=sizeof(sNktBlock_t);

               ++(pCurTrack->currentBlockId);

          } // end delta check

  }else{
    // check sequence state if stopped reset position
    // and tempo to default, but only once

   if(bStopped==FALSE){
      bStopped=TRUE;

      g_CurrentNktSequence->currentTempo.tempo=g_CurrentNktSequence->defaultTempo.tempo;

      //copy/update precalculated tempo data
      for (int i=0;i<NKT_UMAX;++i){
          g_CurrentNktSequence->currentTempo.tuTable[i]=g_CurrentNktSequence->defaultTempo.tuTable[i];
      }

      TimeAdd = 0;

      // reset tempo to initial valueas taken during start (get them from main sequence?)
      // get precalculated timestep
      g_CurrentNktSequence->timeStep=g_CurrentNktSequence->currentTempo.tuTable[g_CurrentNktSequence->currentUpdateFreq];

      //rewind all tracks to the first event
      for(int i=0;i<g_CurrentNktSequence->nbOfTracks;++i){
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

void updateStepNktMt(void){

 // handle master volume, balance, reverb, mt32 text
 handleMasterSettings();

 if(g_CurrentNktSequence==0) return;

 sequenceState=g_CurrentNktSequence->sequenceState;

 //check sequence state if paused do nothing
 if((sequenceState&NKT_PS_PAUSED)){

     if(bPaused==FALSE){
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

      for(int i=0;i<g_CurrentNktSequence->nbOfTracks;++i){

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
      nktBlk=(sNktBlock_t *)(pEventPtr);

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
              pCurTrack->eventsBlockOffset+=sizeof(sNktBlock_t);
              ++(pCurTrack->currentBlockId);

              // get next event block
              addr=((uint32)pCurTrack->eventBlocksPtr)+pCurTrack->eventsBlockOffset;
              uint8 count=0;

              // read VLQ delta
              uint8 *pEventPtr=(uint8 *)(addr);
              uint32 currentDelta = readVLQ(pEventPtr,&count);
              pEventPtr+=count;

              // get event block
              nktBlk=(sNktBlock_t *)(pEventPtr);
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
          pCurTrack->eventsBlockOffset+=sizeof(sNktBlock_t);

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

   if(bStopped==FALSE){
      bStopped=TRUE;

      g_CurrentNktSequence->currentTempo.tempo=g_CurrentNktSequence->defaultTempo.tempo;

      //copy/update precalculated tempo data
      for (int i=0;i<NKT_UMAX;++i){
          g_CurrentNktSequence->currentTempo.tuTable[i]=g_CurrentNktSequence->defaultTempo.tuTable[i];
      }

      TimeAdd = 0;

      TrackEndCount=0;

      // reset tempo to initial valueas taken during start (get them from main sequence?)
      // get precalculated timestep
      g_CurrentNktSequence->timeStep=g_CurrentNktSequence->currentTempo.tuTable[g_CurrentNktSequence->currentUpdateFreq];

      //rewind all tracks to the first event
      for(int i=0;i<g_CurrentNktSequence->nbOfTracks;++i){
          g_CurrentNktSequence->pTracks[i].timeElapsedInt=0L;
          g_CurrentNktSequence->pTracks[i].timeElapsedFrac=0L;
          g_CurrentNktSequence->pTracks[i].eventsBlockOffset=0L;
          g_CurrentNktSequence->pTracks[i].currentBlockId=0;
      }

      resetMidiDevice();

    }
   return;
  }

} //end updateStepNkt()




sNktSeq *loadSequence(const uint8 *pFilePath){
    // create header
    sNktSeq *pNewSeq=(sNktSeq *)gUserMemAlloc(sizeof(sNktSeq),PREFER_TT,0);

    if(pNewSeq==0)
    {
      amTrace("Error: Couldn't allocate memory for sequence header.\n");

#ifndef SUPRESS_CON_OUTPUT
    printf("Error: Couldn't allocate memory for sequence header.\n");
#endif

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

    if(pFilePath){
         // create file header

#ifndef SUPRESS_CON_OUTPUT
        printf("Loading NKT file: %s\n",pFilePath);
#endif

        amTrace("Loading NKT file: %s\n",pFilePath);

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

            #ifndef SUPRESS_CON_OUTPUT
                printf("Error: Couldn't open : %s. File doesn't exists.\n",pFilePath);
            #endif

            amTrace("Error: Couldn't open : %s. File doesn't exists.\n",pFilePath);
            gUserMemFree(pNewSeq,0);
            return NULL;
         }
      }else{

        #ifndef SUPRESS_CON_OUTPUT
        printf("Error: empty file path. Exiting...\n");
        #endif

        amTrace("Error: empty file path. Exiting...\n");
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
          amTrace("[GEMDOS] Error: %s\n",getGemdosError(read));
    }else{
        if(read<sizeof(sNktHd)){
            amTrace("[GEMDOS] Read error, unexpected EOF. Expected: %d, read: %d\n",sizeof(sNktHd),read);
        }
    }

#else
      fread(&tempHd,sizeof(sNktHd),1,fp);
#endif

// check header
    if(tempHd.id!=ID_NKT){

        #ifndef SUPRESS_CON_OUTPUT
            printf("Error: File %s isn't valid!\n",pFilePath);
        #endif

        amTrace("Error: File %s isn't valid!/. Exiting...\n",pFilePath);

        #ifdef ENABLE_GEMDOS_IO
            amTrace("[GEMDOS] Closing file handle : [%d] \n", fh);
            int16 err=Fclose(fh);

            if(err!=GDOS_OK){
              amTrace("[GEMDOS] Error closing file handle : [%d] %s\n", fh, getGemdosError(err));
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
      amTrace("Error: Couldn't allocate memory for track info\n.");
      return NULL;
   }

   amMemSet(trackData,0,sizeof(sNktTrackInfo) * tempHd.nbOfTracks);

#ifdef ENABLE_GEMDOS_IO
    Fseek(sizeof(sNktHd), fh, 0 );
    read=Fread(fh,sizeof(sNktTrackInfo)* tempHd.nbOfTracks,trackData);

    if(read<0){
          // GEMDOS ERROR TODO, display error for now
          amTrace("[GEMDOS] Error: %s\n",getGemdosError(read));
    }else{
        if(read<(sizeof(sNktTrackInfo)* tempHd.nbOfTracks)){
            amTrace("[GEMDOS] Read error, unexpected EOF. Expected: %d, read: %d\n",sizeof(sNktTrackInfo)* tempHd.nbOfTracks,read);
        }
    }

#else
      fread(trackData,sizeof(sNktTrackInfo)* tempHd.nbOfTracks,1,fp);
#endif

   amTrace("[NKT header]\nnb of Tracks: %u ", tempHd.nbOfTracks);
   amTrace("td: %u \n", tempHd.division);

   if(tempHd.version!=NKT_VERSION){
     amTrace("Error: Wrong version of NKT file. Cannot load file.\n");
     gUserMemFree(trackData,0);
     return NULL;
   }

   for (int i=0;i<tempHd.nbOfTracks;++i){
     amTrace("[NKT track #%u]\nNb of blocks: %u (%u bytes),\nEvent data buffer size: %u\n",i,trackData[i].nbOfBlocks, trackData[i].eventsBlockBufSize,trackData[i].eventDataBufSize);
   }

   lzo_voidp pPackedEvents=0;
   lzo_voidp pPackedData=0;
   lzo_uint newEventSize=0;
   lzo_uint newDataSize=0;



   if( trackData[0].nbOfBlocks==0 || trackData[0].eventsBlockBufSize==0 || trackData[0].eventDataBufSize==0 ){

    #ifndef SUPRESS_CON_OUTPUT
        printf("Error: File %s has no data or event blocks!\n",pFilePath);
    #endif

    amTrace("\n Error: File %s has no data or event blocks!\n",pFilePath);

    #ifdef ENABLE_GEMDOS_IO
        amTrace("[GEMDOS] Closing file handle : [%d] \n", fh);
        int16 err=Fclose(fh);

        if(err!=GDOS_OK){
          amTrace("[GEMDOS] Error closing file handle : [%d] %s\n", fh, getGemdosError(err));
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
            amTrace("Error: Couldn't allocate memory for track info.\n");

            gUserMemFree(pNewSeq,0);
            return NULL;
        }

        // ok
        for(int i=0;i<pNewSeq->nbOfTracks;++i){

            sNktTrack *pTrk=(sNktTrack *)&pNewSeq->pTracks[i];

            pTrk->currentBlockId=0;
            pTrk->eventsBlockOffset=0;

            pTrk->nbOfBlocks = trackData[i].nbOfBlocks;
            pTrk->eventsBlockBufferSize = trackData[i].eventsBlockBufSize;
            pTrk->dataBufferSize = trackData[i].eventDataBufSize;

            amTrace("\n[Track]: %u \n", i);
            amTrace("Blocks in sequence: %u \n", pTrk->nbOfBlocks);
            amTrace("Event data buffer size: %u \n", pTrk->dataBufferSize);
            amTrace("Events block size: %u \n", pTrk->eventsBlockBufferSize);


        // packed data check
        if(trackData[i].eventsBlockBufSize!=trackData[i].eventsBlockPackedSize){
            amTrace("[Data packed]\n");

            lzo_voidp pPackedDataSource=0;

            if(lzo_init()!=LZO_E_OK){
              amTrace("Error: Could't initialise LZO library. Cannot depack data. \n");
              return NULL;
            }

            amTrace("[LZO] LZO real-time data compression library (v%s, %s).\n",
                   lzo_version_string(), lzo_version_date());

                // unpack them to temp buffers

                amTrace("[LZO] Allocating temp events buffer\n");
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
                        amTrace("[GEMDOS] Error: read %lu, expected: %lu \n",read,pTrk->eventsBlockBufferSize);
                    }else
                        amTrace("[GEMDOS] Read events buffer data %lu \n",read);
#else

#error TODO

#endif
                    amTrace("[LZO] Decompressing events block...\n");
                    int decResult=lzo1x_decompress(pPackedDataSource,pTrk->eventsBlockBufferSize,pPackedEvents,&newEventSize,LZO1X_MEM_DECOMPRESS);

                    if( decResult == LZO_E_OK){
                        amTrace("[LZO] Decompressed events buffer %ld->%ld bytes\n",pTrk->eventsBlockBufferSize,newEventSize);
                        pTrk->eventsBlockBufferSize=newEventSize;
                    }else{
                        amTrace("[LZO] Error: Events block decompression error: %d at %ld\n",decResult,newEventSize);
                    }

                    //release
                    gUserMemFree(pPackedDataSource,0);
                }

                amTrace("[LZO] Allocating temp data buffer\n");
                amount = pTrk->dataBufferSize*3;

                pPackedData=(lzo_voidp)gUserMemAlloc(amount,PREFER_TT,0);

                pPackedDataSource=(lzo_voidp)gUserMemAlloc(pTrk->dataBufferSize,PREFER_TT,0);


                if(pPackedData!=NULL && pPackedDataSource!=NULL){
                    amMemSet(pPackedData,0,amount);
                    amMemSet(pPackedDataSource,0,pTrk->dataBufferSize);

#ifdef ENABLE_GEMDOS_IO
                    amTrace("[GEMDOS] Read events data buffer\n");
                    int32 read=Fread(fh, pTrk->dataBufferSize, pPackedDataSource);
#else
#error TODO
#endif
                    amTrace("[LZO] Decompressing data block...\n");
                    int decResult = lzo1x_decompress(pPackedDataSource,pTrk->dataBufferSize,pPackedData,&newDataSize,LZO1X_MEM_DECOMPRESS);

                    if( decResult == LZO_E_OK){
                        amTrace("[LZO] Decompressed events buffer %ld->%ld bytes\n",pTrk->dataBufferSize,newDataSize);
                        pTrk->dataBufferSize=newDataSize;
                    }else{
                        amTrace("[LZO] Error: Data block decompression error: %d at %ld\n", decResult, newDataSize);
                    }

                    //release
                    gUserMemFree(pPackedDataSource,0);
                }

        }

        uint32 lbAllocAdr=0;

        // create linear buffer
        if(createLinearBuffer(&(pTrk->lbEventsBuffer),pTrk->eventsBlockBufferSize+255,PREFER_TT)<0){

            #ifndef SUPRESS_CON_OUTPUT
            printf("Error: loadSequence() Couldn't allocate memory for event block buffer.\n");
            #endif

            amTrace("Error: loadSequence() Couldn't allocate memory for event block buffer.\n");

            #ifdef ENABLE_GEMDOS_IO
                amTrace("[GEMDOS] Closing file handle : [%d] \n", fh);

                int16 err=Fclose(fh);

                if(err!=GDOS_OK){
                  amTrace("[GEMDOS] Error closing file handle : [%d] %s\n", fh, getGemdosError(err));
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

#ifndef SUPRESS_CON_OUTPUT
            printf("Error: loadSequence() Linear buffer out of memory.\n");
#endif
            amTrace("Error: loadSequence() Linear buffer out of memory.\n");

            #ifdef ENABLE_GEMDOS_IO
                amTrace("[GEMDOS] Closing file handle : [%d] \n", fh);

                int16 err=Fclose(fh);

                if(err!=GDOS_OK){
                  amTrace("[GEMDOS] Error closing file handle : [%d] %s\n", fh, getGemdosError(err));
                }

            #else
                fclose(fp); fp=0;
            #endif

             gUserMemFree(pNewSeq,0);
             return NULL;
         }

         amTrace("Allocated %lu k for event block buffer\n",pTrk->eventsBlockBufferSize);

         if(createLinearBuffer(&(pTrk->lbDataBuffer),pTrk->dataBufferSize+255,PREFER_TT)<0){

            #ifndef SUPRESS_CON_OUTPUT
                printf("Error: loadSequence() Couldn't allocate memory for temp data buffer. \n");
            #endif

             amTrace("Error: loadSequence() Couldn't allocate memory for temp data buffer. \n");

            #ifdef ENABLE_GEMDOS_IO
                amTrace("[GEMDOS] Closing file handle : [%d] \n", fh);

                int16 err=Fclose(fh);

                if(err!=GDOS_OK){
                  amTrace("[GEMDOS] Error closing file handle : [%d] %s\n", fh, getGemdosError(err));
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
       amTrace("Allocated %lu b for event data buffer\n",pTrk->dataBufferSize);

       if(pTrk->eventDataPtr==0){

           #ifndef SUPRESS_CON_OUTPUT
                printf("Error: loadSequence() Linear buffer out of memory.\n");
           #endif

           amTrace("Error: loadSequence() Linear buffer out of memory.\n");

           // destroy block buffer
           destroyLinearBuffer(&(pTrk->lbEventsBuffer));
           destroyLinearBuffer(&(pTrk->lbDataBuffer));

           #ifdef ENABLE_GEMDOS_IO
            amTrace("[GEMDOS] Closing file handle : [%d] \n", fh);

            int16 err=Fclose(fh);

            if(err!=GDOS_OK){
                amTrace("[GEMDOS] Error closing file handle : [%d] %s\n", fh, getGemdosError(err));
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
                 amTrace("[GEMDOS] error, read bytes: %ld, expected read: %ld ",read,amount);
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
                amTrace("[GEMDOS] error, read bytes: %ld, expected read: %ld ",read,amount);
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
    amTrace("[LOAD TEST]\n");
    uint32 blockNb=0;
    uint8 count=0;

   for(int i=0;i<pNewSeq->nbOfTracks;++i){
    amTrace("[Track #%u]\n",i);

    uint32 blockNb=0;
    uint8 count=0;

    while(blockNb<pNewSeq->pTracks[i].nbOfBlocks){

      uint32 addr=((uint32)pNewSeq->pTracks[i].eventBlocksPtr) + pNewSeq->pTracks[i].eventsBlockOffset;

      uint8 *pEventPtr=(uint8 *)(addr);
      uint32 d=readVLQ(pEventPtr,&count);

      pEventPtr+=count;

      sNktBlock_t *eBlk=(sNktBlock_t *)(pEventPtr);

      amTrace("delta [%lu] type:[%hu] size:[%hu] bytes offset: [%lu] \n",d, eBlk->msgType, eBlk->blockSize,eBlk->bufferOffset);

      if(eBlk->blockSize>0){
          amTrace("[DATA] ");

          uint8 *data = (uint8 *)((uint32)(pNewSeq->pTracks[i].eventDataPtr)+eBlk->bufferOffset);

          for(int j=0;j<eBlk->blockSize;++j){
           amTrace("0x%02x ",data[j]);
          }

          amTrace(" [/DATA]\n");
      }

      pNewSeq->pTracks[i].eventsBlockOffset+=count;
      pNewSeq->pTracks[i].eventsBlockOffset+=sizeof(sNktBlock_t);

      ++blockNb;
    }
   }
#endif



// close file
#ifdef ENABLE_GEMDOS_IO

    amTrace("[GEMDOS] Closing file handle : [%d] \n", fh);
    int16 err=Fclose(fh);

    if(err!=GDOS_OK){
        amTrace("[GEMDOS] Error closing file handle : [%d] %s\n", fh, getGemdosError(err));
    }

#else
    fclose(fp);fp=0;
#endif

 return pNewSeq;
}

void destroySequence(sNktSeq *pSeq){

  if(pSeq==0) return;

  if(pSeq->pTracks){

   for(int i=0; i<pSeq->nbOfTracks; ++i){

        if(pSeq->pTracks[i].nbOfBlocks!=0){

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

bool isSequencePlaying(void){

 if(g_CurrentNktSequence!=0){
     uint16 state=g_CurrentNktSequence->sequenceState;
     if((state&NKT_PS_PLAYING)&&(!(state&NKT_PS_PAUSED)))
        return TRUE;
       else
        return FALSE;
 }

 return FALSE;
}


void stopSequence(void){
 if(g_CurrentNktSequence!=0){
  uint16 state=g_CurrentNktSequence->sequenceState;

  if((state&NKT_PS_PLAYING)||(state&NKT_PS_PAUSED)){
       g_CurrentNktSequence->sequenceState&=(~(NKT_PS_PLAYING|NKT_PS_PAUSED));

#ifndef SUPRESS_CON_OUTPUT
       printf("Stop sequence\n");
#endif

    setMT32Message("Stopped...");

  }

    resetMidiDevice();

  }
}

void pauseSequence(){

     if(g_CurrentNktSequence!=0){
        uint16 state=g_CurrentNktSequence->sequenceState;

          if((state&NKT_PS_PLAYING)&&(!(state&NKT_PS_PAUSED))){
           g_CurrentNktSequence->sequenceState&=(~NKT_PS_PLAYING);
           g_CurrentNktSequence->sequenceState|=NKT_PS_PAUSED;

#ifndef SUPRESS_CON_OUTPUT
           printf("Pause sequence\n");
#endif
           setMT32Message("Paused...");

           return;
          }else if(!(state&NKT_PS_PLAYING)&&(state&NKT_PS_PAUSED) ){
            g_CurrentNktSequence->sequenceState&=(~NKT_PS_PAUSED); //unpause
            g_CurrentNktSequence->sequenceState|=NKT_PS_PLAYING;  //set playing state

            setMT32Message("Playing...");

          }
      }
 } //pauseSequence

// play sequence
void playSequence(void){

if(g_CurrentNktSequence!=0){
  uint16 state=g_CurrentNktSequence->sequenceState;

    if(!(state&NKT_PS_PLAYING)){

         g_CurrentNktSequence->sequenceState&=(~(NKT_PS_PAUSED));
         g_CurrentNktSequence->sequenceState|=NKT_PS_PLAYING;

#ifndef SUPRESS_CON_OUTPUT
         printf("Play sequence\t");


         if(g_CurrentNktSequence->sequenceState&NKT_PLAY_ONCE){

           printf("[ ONCE ]\n");

         }else{

           printf("[ LOOP ]\n");

         }
#endif

      }
 }
}

void switchReplayMode(void){

 if(g_CurrentNktSequence!=0){
     if(g_CurrentNktSequence->sequenceState&NKT_PLAY_ONCE){
        g_CurrentNktSequence->sequenceState&=(~NKT_PLAY_ONCE);

#ifndef SUPRESS_CON_OUTPUT
        printf("Set replay mode: [ LOOP ]\n");
#endif

     }else{
        g_CurrentNktSequence->sequenceState|=NKT_PLAY_ONCE;

#ifndef SUPRESS_CON_OUTPUT
        printf("Set replay mode: [ ONCE ]\n");
#endif

     }
  }
}

extern void installMidiResetHandler(void);

void NktInit(const eMidiDeviceType devType, const uint8 channel)
{

    amSetDefaultUserMemoryCallbacks();
    
    initDebug("NKTLOG.LOG");

    Supexec(installMidiResetHandler);

    // now depending on the connected device type and chosen operation mode
    // set appropriate channel
    // prepare device for receiving messages

    setupMidiDevice(devType,channel);
    setMT32Message("AMIDILIB init...");
}


void NktDeinit(){
  setMT32Message("Bye ! ;-)");
  deinitDebug();
}

#ifdef DEBUG_BUILD

// debug stuff
static const uint8 *getSequenceStateStr(const uint16 state){

 if( !(state&NKT_PS_PLAYING) && (state&NKT_PS_PAUSED) ){
    return "Paused";
 }else if(state&NKT_PS_PLAYING && (!(state&NKT_PS_PAUSED))){
    return "Playing";
 }else if(!(state&NKT_PS_PLAYING)){
    return "Stopped...";
 }
}


void printNktSequenceState(void){

#ifndef SUPRESS_CON_OUTPUT

if(g_CurrentNktSequence){
    printf("Td/PPQN: %u\n",g_CurrentNktSequence->timeDivision);
    printf("Time step: %lu\n",g_CurrentNktSequence->timeStep);

    for(int i=0;i<g_CurrentNktSequence->nbOfTracks;++i){
        printf("[%d] Time elapsedFrac: %lu\t",i,g_CurrentNktSequence->pTracks[i].timeElapsedFrac);
        printf("\tTime elapsed: %lu\n",g_CurrentNktSequence->pTracks[i].timeElapsedInt);
    }

    printf("\tDefault Tempo: %lu\n",g_CurrentNktSequence->defaultTempo.tempo);
    printf("\tLast Tempo: %lu\n",g_CurrentNktSequence->currentTempo.tempo);

    printf("\tSequence state: %s\n",getSequenceStateStr(g_CurrentNktSequence->sequenceState));
  }

 printMidiSendBufferState();

#endif

}

static const uint8* _arNktEventName[NKT_MAX_EVENT]={
    "NKT_MIDIDATA",
    "NKT_TEMPO_CHANGE",
    "NKT_JUMP",
    "NKT_TRIGGER",
    "NKT_END"
};

const uint8 *getEventTypeName(uint16 type){
    switch(type){
        case NKT_MIDIDATA: return _arNktEventName[0]; break;
        case NKT_TEMPO_CHANGE: return _arNktEventName[1]; break;
        case NKT_JUMP: return _arNktEventName[2]; break;
        case NKT_TRIGGER: return _arNktEventName[3]; break;
        case NKT_END: return _arNktEventName[4]; break;
        default: return 0;

    }
}
#endif


int32 saveEventDataBlocks(int16 fh, sNktSeq *pSeq){

        // save data blocks
        for(int i=0;i<pSeq->nbOfTracks;++i){
            int32 written=0;

            // save event block
            amTrace("[MID2NKT] Saving event block.[%ld bytes] for track [%d] \n",pSeq->pTracks[i].eventsBlockBufferSize, i);

            written = Fwrite(fh,pSeq->pTracks[i].eventsBlockBufferSize,(void *)pSeq->pTracks[i].eventBlocksPtr);

            if(written<pSeq->pTracks[i].eventsBlockBufferSize){
               amTrace("[GEMDOS]Fatal error: Events block write error, written: %ld , expected %ld bytes\n", written, pSeq->pTracks[i].eventsBlockBufferSize);
               amTrace("[GEMDOS] Error: %s\n", getGemdosError((int16)written));

               return -1;
            }else{
                amTrace("[GEMDOS] written: %ld bytes\n", written);

            }

            // save data block
            amTrace("[MID2NKT] Saving data block.[%ld bytes] for track [%d]\n",pSeq->pTracks[i].dataBufferSize, i);

            written=Fwrite(fh,pSeq->pTracks[i].dataBufferSize,(void *)pSeq->pTracks[i].eventDataPtr);

            if(written<pSeq->pTracks[i].dataBufferSize)
            {
               amTrace("[GEMDOS]Fatal error: Event data block write error, written: %ld , expected %ld bytes\n", written, pSeq->pTracks[i].dataBufferSize);
               amTrace("[GEMDOS] Error: %s\n", getGemdosError((int16)written));

               return -1;
            }else{
                amTrace("[GEMDOS] written: %ld bytes\n", written);
            }

        }

    return 0;
}



int32 saveSequence(sNktSeq *pSeq,const uint8 *filepath, Bool bCompress){

if(filepath==0||strlen(filepath)==0) {
    amTrace("[MID2NKT] Fatal error, path is empty.\n");
    return 0;
}

// create header
sNktHd nktHd;
sNktTrackInfo *pTrackInfo=0;

if(pSeq->nbOfTracks==0){
    amTrace("[MID2NKT] Fatal error, no tracks in sequence!\n");
    return 0;
}

pTrackInfo=(sNktTrackInfo *)gUserMemAlloc( (sizeof(sNktTrackInfo) * pSeq->nbOfTracks), PREFER_TT, 0);

if(pTrackInfo==0) {
    amTrace("[MID2NKT] Fatal error, no memory for track info!\n");
    return 0;
}

// set header
amTrace("[MID2NKT] Init header...\n");

setNktHeader(&nktHd, pSeq);
setNktTrackInfo(pTrackInfo,pSeq);

#ifdef ENABLE_GEMDOS_IO
 int16 fh=GDOS_INVALID_HANDLE;

 amTrace("[GEMDOS] Save sequence to %s, compress: %d\n",filepath, bCompress);

 // file create
 fh = Fcreate(filepath,0);

 if(fh<0){
     amTrace("[GEMDOS] Error: %s\n", getGemdosError(fh));
     return -1;
 }

     if(bCompress!=FALSE){

          amTrace("[MID2NKT] LZO compression ...\n");

         if(lzo_init()!=LZO_E_OK){
           amTrace("Error: Could't initialise LZO library. \n");
           return -1;
         }

          amTrace("[LZO] \nLZO real-time data compression library (v%s, %s).\n",
                 lzo_version_string(), lzo_version_date());

          // allocate work buffers
          uint32 workMemSize=(LZO1X_1_MEM_COMPRESS + (sizeof(lzo_align_t)-1)/sizeof(lzo_align_t))*sizeof(lzo_align_t);

          amTrace("[LZO] Allocating work buffer: %ld bytes\n",workMemSize);

          lzo_voidp workMem=(lzo_voidp)gUserMemAlloc(workMemSize,PREFER_TT,0); // lzo work buffer

          if(workMem!=0){

              amMemSet(workMem,0,workMemSize);

              amTrace("[LZO] Compressing events block.\n");
              MemSize tempBufSize=(pSeq->pTracks[0].eventsBlockBufferSize+pSeq->pTracks[0].eventsBlockBufferSize/16+64+3);
              lzo_bytep tempBuffer=(lzo_bytep)gUserMemAlloc(tempBufSize,PREFER_TT,0);

              if(tempBuffer==NULL){
                  amTrace("[LZO] Error, no memory for events block output buffer.\n");

                  // free work mem
                  gUserMemFree(workMem,0);
                  return -1;
              }else{
                  amMemSet(tempBuffer,0,tempBufSize);
              }

              // compress
              lzo_uint nbBytesPacked=0;
              if(lzo1x_1_compress(pSeq->pTracks[0].eventBlocksPtr,pSeq->pTracks[0].eventsBlockBufferSize,tempBuffer,&nbBytesPacked,workMem)==LZO_E_OK){
                    amTrace("[LZO] Event data compressed %lu->%lu bytes.\n",pSeq->pTracks[0].eventsBlockBufferSize, nbBytesPacked);

                    /* check for an incompressible block */
                    if (nbBytesPacked >= pSeq->pTracks[0].eventsBlockBufferSize){
                            amTrace("[LZO] Error: Event block contains incompressible data.\n");
                        return -1;
                    }

                    // copy output buffer with packed data
                    pSeq->pTracks[0].eventsBlockBufferSize = nbBytesPacked;
                    pTrackInfo[0].eventsBlockPackedSize = nbBytesPacked;
                    amMemCpy(pSeq->pTracks[0].eventBlocksPtr,tempBuffer,nbBytesPacked);

              }else{
                 amTrace("[LZO] Internal error: Compression failed.\n");
              }

              gUserMemFree(tempBuffer,0);

              amTrace("[LZO] Compressing data block.\n");
              tempBufSize=pSeq->pTracks[0].dataBufferSize+pSeq->pTracks[0].dataBufferSize/16+64+3;
              tempBuffer=(lzo_bytep)gUserMemAlloc(tempBufSize,PREFER_TT,0);

              if(tempBuffer==NULL){
                  amTrace("[LZO] Error, no memory for data block output buffer.\n");

                  // free work mem
                  gUserMemFree(workMem,0);
                  return -1;
              }else{
                  amMemSet(tempBuffer,0,tempBufSize);
              }

              // compress data block
              nbBytesPacked=0;
              if(lzo1x_1_compress(pSeq->pTracks[0].eventDataPtr,pSeq->pTracks[0].dataBufferSize,tempBuffer,&nbBytesPacked,workMem)==LZO_E_OK){
                    amTrace("[LZO] Data block compressed %lu->%lu bytes.\n",pSeq->pTracks[0].dataBufferSize, nbBytesPacked);

                    /* check for an incompressible block */
                      if (nbBytesPacked >= pSeq->pTracks[0].dataBufferSize){
                            amTrace("[LZO] Error: Data block contains incompressible data.\n");
                        return -1;
                      }

                    // copy output buffer with packed data
                    // TODO: shrink existing buffer somehow

                    pSeq->pTracks[0].dataBufferSize=nbBytesPacked;
                    pTrackInfo[0].eventDataBlockPackedSize = nbBytesPacked;

                    amMemCpy(pSeq->pTracks[0].eventDataPtr,tempBuffer,nbBytesPacked);

              }else{
                  amTrace("[LZO] Internal error: Compression failed.\n");
              }

              //copy output buffer with packed data to
              gUserMemFree(tempBuffer,0);

              // free work mem
              gUserMemFree(workMem,0);

          }else{
              amTrace("[LZO] Error couldn't allocate compression work memory.\n");
              return -1;
          }

          // save header
          int32 written=0;

          // save header
          amTrace("[MID2NKT] Saving header... \n");
          written=Fwrite(fh, sizeof(sNktHd), &nktHd);

          if(written<sizeof(sNktHd)){
             amTrace("[GEMDOS]Fatal error: Header write error, written: %ld, expected %ld\n", written, sizeof(sNktHd));
             amTrace("[GEMDOS] Error: %s\n", getGemdosError((int16)written));
             return -1;
          }else{
              amTrace("[GEMDOS] written: %ld bytes\n", written);
          }

          // save track data
          written=Fwrite(fh, sizeof(sNktTrackInfo) * pSeq->nbOfTracks, pTrackInfo);

          if(written<sizeof(sizeof(sNktTrackInfo) * pSeq->nbOfTracks)){
             amTrace("[GEMDOS]Fatal error: Track write error, written: %ld, expected %ld\n", written, sizeof(sNktTrackInfo) * pSeq->nbOfTracks);
             amTrace("[GEMDOS] Error: %s\n", getGemdosError((int16)written));
             return -1;
          }else{
              amTrace("[GEMDOS] written: %ld bytes\n", written);
          }


          // write data / event blocks
          if(saveEventDataBlocks(fh,pSeq)<0){
              return -1;
          }

     }else{

         amTrace("[GEMDOS] Created file handle: %d\n",fh);

         int32 written=0;

         // save header
         amTrace("[MID2NKT] Saving header... \n");
         written=Fwrite(fh, sizeof(sNktHd), &nktHd);

         if(written<sizeof(sNktHd)){
            amTrace("[GEMDOS]Fatal error: Header write error, written: %ld, expected %ld\n", written, sizeof(sNktHd));
            amTrace("[GEMDOS] Error: %s\n", getGemdosError((int16)written));
            return -1;
         }else{
             amTrace("[GEMDOS] written: %ld bytes\n", written);
         }

         // save track data
         written=Fwrite(fh, sizeof(sNktTrackInfo) * pSeq->nbOfTracks, pTrackInfo);

         if(written<sizeof(sizeof(sNktTrackInfo) * pSeq->nbOfTracks)){
            amTrace("[GEMDOS]Fatal error: Track write error, written: %ld, expected %ld\n", written, sizeof(sNktTrackInfo) * pSeq->nbOfTracks);
            amTrace("[GEMDOS] Error: %s\n", getGemdosError((int16)written));
            return -1;
         }else{
             amTrace("[GEMDOS] written: %ld bytes\n", written);
         }

         // write data / event blocks
         if(saveEventDataBlocks(fh,pSeq)<0){
             return -1;
         }

     }

     // close file
     int16 err = Fclose(fh);

     if(err!=GDOS_OK){
       amTrace("[GEMDOS] Error closing file handle : [%d] %s\n", fh, getGemdosError(err));
       return -1;
     }else{
         amTrace("[GEMDOS] Closed file handle : [%d] \n", fh);
     }


#else
FILE *file;
 amTrace("Save sequence to %s, compress: %d\n",filepath,bCompress);

 // file create
 // save header
 // save track data

 // for each track:
 //// save event block
 //// save data block

 // close file

#endif

 return 0;
}


void setNktHeader(sNktHd* header, const sNktSeq *pNktSeq){

    if(header){
        // clear header
        amMemSet(header, 0L, sizeof(sNktHd));
        header->id=ID_NKT;
        header->nbOfTracks=pNktSeq->nbOfTracks;
        header->division = pNktSeq->timeDivision;
        header->version = NKT_VERSION;
    }
}

void setNktTrackInfo(sNktTrackInfo* trackInfo, const sNktSeq *pNktSeq){

    if(trackInfo){

        for(uint16 i=0;i<pNktSeq->nbOfTracks;++i){
            trackInfo[i].nbOfBlocks = pNktSeq->pTracks[i].nbOfBlocks;
            trackInfo[i].eventDataBlockPackedSize = trackInfo[i].eventDataBufSize = pNktSeq->pTracks[i].dataBufferSize;
            trackInfo[i].eventsBlockPackedSize = trackInfo[i].eventsBlockBufSize = pNktSeq->pTracks[i].eventsBlockBufferSize;
            trackInfo[i].nbOfBlocks = pNktSeq->pTracks[i].nbOfBlocks;

            amTrace("Set track [%d]: event data buffer: %ld events block buffer: %ld\n", i, trackInfo[i].eventDataBufSize,trackInfo[i].eventsBlockBufSize);
        }

    }
}

