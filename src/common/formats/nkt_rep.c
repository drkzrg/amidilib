
/**  Copyright 2007-2014 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
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

#include "minilzo.h" //lzo depack, TODO: add compilation flag to remove lzo during compilation time

#ifdef ENABLE_GEMDOS_IO
#include "fmio.h"
#include <mint/ostruct.h>
#include <mint/osbind.h>
#endif

static sNktSeq *g_CurrentNktSequence=0;

void getCurrentSequence(sNktSeq **pSeq){
  *pSeq=g_CurrentNktSequence;
}


// this is called when sequence ends in TiC callback
static void onEndSequence(){

if(g_CurrentNktSequence){

  if(g_CurrentNktSequence->sequenceState&NKT_PLAY_ONCE){
    // set state to stopped
    // reset song position on all tracks
    g_CurrentNktSequence->sequenceState&=(U16)(~(NKT_PS_PLAYING|NKT_PS_PAUSED));
    am_allNotesOff(16);

#ifdef IKBD_MIDI_SEND_DIRECT
  flushMidiSendBuffer();
#endif

  }else{
    // loop
    g_CurrentNktSequence->sequenceState&=(U16)(~NKT_PS_PAUSED);
    g_CurrentNktSequence->sequenceState|=(U16)NKT_PS_PLAYING;
  }

  g_CurrentNktSequence->timeElapsedInt=0L;
  g_CurrentNktSequence->timeElapsedFrac=0L;

  g_CurrentNktSequence->currentTempo.tempo=g_CurrentNktSequence->defaultTempo.tempo;
  g_CurrentNktSequence->currentBlockId=0l;
  g_CurrentNktSequence->eventsBlockOffset=0L;

  // reset all tracks state
  g_CurrentNktSequence->timeStep=g_CurrentNktSequence->defaultTempo.tuTable[g_CurrentNktSequence->currentUpdateFreq];


 }

}

// init sequence
void initSequence(sNktSeq *pSeq, U16 initialState, BOOL bInstallUpdate){
 g_CurrentNktSequence=0;

if(pSeq!=0){
    g_CurrentNktSequence=pSeq;

    pSeq->currentUpdateFreq=NKT_U200HZ;
    pSeq->currentTempo.tempo = DEFAULT_MPQN;

    U32 td=pSeq->timeDivision;
    U32 bpm = DEFAULT_BPM;
    U32 tempPPU = bpm * td;

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
                  assert(0);
                  amTrace((const U8*)"[Error] Invalid timer update value %d\n", i);
              } break;
          };

     } //end for

    pSeq->timeElapsedInt=0UL;
    pSeq->timeElapsedFrac=0UL;
    pSeq->currentBlockId=0;
    pSeq->eventsBlockOffset=0L;
    pSeq->timeStep=pSeq->defaultTempo.tuTable[pSeq->currentUpdateFreq];

    pSeq->sequenceState = initialState;


    setMidiMasterVolume(64);

#ifdef IKBD_MIDI_SEND_DIRECT
    Supexec(clearMidiOutputBuffer);
#endif

    // install our interrupt handler
if(bInstallUpdate!=FALSE) Supexec(NktInstallReplayRout);

#ifdef DEBUG_BUILD
  printNktSequenceState();
#endif

  } //endif
 return;
}

#ifdef DEBUG_BUILD
void initSequenceManual(sNktSeq *pSeq, U16 state){
 g_CurrentNktSequence=0;

 if(pSeq!=0){
  U8 mode=0,data=0;
  g_CurrentNktSequence=pSeq;

  pSeq->currentTempo.tempo = pSeq->defaultTempo.tempo;

  pSeq->timeElapsedInt = 0UL;
  pSeq->timeElapsedFrac = 0UL;
  pSeq->currentBlockId = 0;
  pSeq->eventsBlockOffset=0L;

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

volatile static BOOL bStopped=FALSE;
volatile static U32 TimeAdd=0;
volatile static U32 addr;
volatile static sNktBlock_t *nktBlk=0;
volatile static U8 MasterVolume;

volatile static U8 currentMasterVolume;
volatile static U8 requestedMasterVolume;
volatile static U16 sequenceState;


void updateStepNkt(){

 if(g_CurrentNktSequence==0) return;

 sequenceState=g_CurrentNktSequence->sequenceState;

/* if(currentMasterVolume!=requestedMasterVolume){
     currentMasterVolume=requestedMasterVolume;

     // send master volume data
     U8 setMasterVolMsg[8]={0xF0,0x7F,0x7F,0x04,0x01,0x00,0x00,0xF7};

     setMasterVolMsg[5]=currentMasterVolume&0x0f; // 0xLL  Bits 0 to 6 of a 14-bit volume
     setMasterVolMsg[6]=currentMasterVolume&0xf0; // 0xMM  Bits 7 to 13 of a 14-bit volume

     MIDIsendBuffer[MIDIbytesToSend++]=setMasterVolMsg[0];
     MIDIsendBuffer[MIDIbytesToSend++]=setMasterVolMsg[1];
     MIDIsendBuffer[MIDIbytesToSend++]=setMasterVolMsg[2];
     MIDIsendBuffer[MIDIbytesToSend++]=setMasterVolMsg[3];
     MIDIsendBuffer[MIDIbytesToSend++]=setMasterVolMsg[4];
     MIDIsendBuffer[MIDIbytesToSend++]=setMasterVolMsg[5];
     MIDIsendBuffer[MIDIbytesToSend++]=setMasterVolMsg[6];
     MIDIsendBuffer[MIDIbytesToSend++]=setMasterVolMsg[7];

 }*/

 //check sequence state if paused do nothing
 if(sequenceState&NKT_PS_PAUSED){
    am_allNotesOff(16);

    #ifdef IKBD_MIDI_SEND_DIRECT
        flushMidiSendBuffer();
    #endif
    return;
  }

  if(sequenceState&NKT_PS_PLAYING){

      bStopped=FALSE;   // we replaying, so we have to reset this flag
      addr=((U32)g_CurrentNktSequence->eventBlocksPtr)+g_CurrentNktSequence->eventsBlockOffset;
      U8 count=0;

      // read VLQ delta
      U8 *pEventPtr=(U8 *)(addr);
      U32 currentDelta=readVLQ(pEventPtr,&count);
      pEventPtr+=count;

      // get event block
      nktBlk=(sNktBlock_t *)(pEventPtr);

      // track end?
      if(nktBlk->msgType&NKT_END||g_CurrentNktSequence->currentBlockId>=g_CurrentNktSequence->nbOfBlocks){
         onEndSequence();
         return;
     }

     // update
     g_CurrentNktSequence->timeElapsedFrac += g_CurrentNktSequence->timeStep;
     TimeAdd = g_CurrentNktSequence->timeElapsedFrac >> 16;
     g_CurrentNktSequence->timeElapsedFrac &= 0xffff;

     // timestep forward
     g_CurrentNktSequence->timeElapsedInt=g_CurrentNktSequence->timeElapsedInt+TimeAdd;

     if( g_CurrentNktSequence->timeElapsedInt==currentDelta||currentDelta==0){
         g_CurrentNktSequence->timeElapsedInt -= currentDelta;

         // tempo change ?
         if(nktBlk->msgType&NKT_TEMPO_CHANGE){
            // set new tempo
            addr=((U32)g_CurrentNktSequence->eventDataPtr)+nktBlk->bufferOffset;
            U32 *pMidiDataStartAdr=(U32 *)(addr);

            g_CurrentNktSequence->currentTempo.tempo=*pMidiDataStartAdr;
            pMidiDataStartAdr++;

            // get precalculated timestep from data buffer
            g_CurrentNktSequence->timeStep=pMidiDataStartAdr[g_CurrentNktSequence->currentUpdateFreq];

            //next event
            g_CurrentNktSequence->eventsBlockOffset+=count;
            g_CurrentNktSequence->eventsBlockOffset+=sizeof(sNktBlock_t);
            ++(g_CurrentNktSequence->currentBlockId);

            // get next event block
            addr=((U32)g_CurrentNktSequence->eventBlocksPtr)+g_CurrentNktSequence->eventsBlockOffset;
            U8 count=0;

            // read VLQ delta
            U8 *pEventPtr=(U8 *)(addr);
            U32 currentDelta=readVLQ(pEventPtr,&count);
            pEventPtr+=count;

            // get event block
            nktBlk=(sNktBlock_t *)(pEventPtr);
        }

         U32 *pMidiDataStartAdr=(U32 *)(((U32)g_CurrentNktSequence->eventDataPtr)+nktBlk->bufferOffset);

#ifdef IKBD_MIDI_SEND_DIRECT
          amMemCpy(MIDIsendBuffer, pMidiDataStartAdr, nktBlk->blockSize);
          MIDIbytesToSend=nktBlk->blockSize;
  #else
          //send to xbios
          amMidiSendData(nktBlk->blockSize, pMidiDataStartAdr);
  #endif

          //go to next event
          g_CurrentNktSequence->eventsBlockOffset+=count;
          g_CurrentNktSequence->eventsBlockOffset+=sizeof(sNktBlock_t);

          ++(g_CurrentNktSequence->currentBlockId);

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

      g_CurrentNktSequence->timeElapsedInt=0L;
      g_CurrentNktSequence->timeElapsedFrac=0L;
      g_CurrentNktSequence->eventsBlockOffset=0L;
      TimeAdd = 0;

      // reset tempo to initial valueas taken during start (get them from main sequence?)
      // get precalculated timestep
      g_CurrentNktSequence->timeStep=g_CurrentNktSequence->currentTempo.tuTable[g_CurrentNktSequence->currentUpdateFreq];

      //rewind to the first event
      g_CurrentNktSequence->currentBlockId=0;
    }
   return;
  }

} //end updateStepNkt()




sNktSeq *loadSequence(const U8 *pFilePath){
    // create header
    sNktSeq *pNewSeq=(sNktSeq *)amMallocEx(sizeof(sNktSeq),PREFER_TT);

    if(pNewSeq==0){
      amTrace("Error: Couldn't allocate memory for sequence header.\n");

#ifndef SUPRESS_CON_OUTPUT
    printf("Error: Couldn't allocate memory for sequence header.\n");
#endif

      return NULL;
    }

    amMemSet(pNewSeq,0,sizeof(sNktSeq));
	
    pNewSeq->currentUpdateFreq=NKT_U200HZ;
    pNewSeq->sequenceState |= NKT_PLAY_ONCE;
    pNewSeq->defaultTempo.tempo=DEFAULT_MPQN;
    pNewSeq->currentTempo.tempo=DEFAULT_MPQN;
    pNewSeq->timeDivision=DEFAULT_PPQN;

    //get nb of blocks from file
#ifdef ENABLE_GEMDOS_IO
    S16 fh=GDOS_INVALID_HANDLE;
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
            amFree(pNewSeq);
            return NULL;
         }
      }else{

        #ifndef SUPRESS_CON_OUTPUT
        printf("Error: empty file path. Exiting...\n");
        #endif

        amTrace("Error: empty file path. Exiting...\n");
        amFree(pNewSeq);
        return NULL;
      }

    // read header
    sNktHd tempHd;
    amMemSet(&tempHd,0,sizeof(sNktHd));

#ifdef ENABLE_GEMDOS_IO
    S32 read=Fread(fh,sizeof(sNktHd),&tempHd);

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
            S16 err=Fclose(fh);

            if(err!=GDOS_OK){
              amTrace("[GEMDOS] Error closing file handle : [%d] \n", fh, getGemdosError(err));
            }

        #else
            fclose(fp); fp=0;
        #endif

     amFree(pNewSeq);
     return NULL;
   }

   amTrace("[NKT header]\nNb of blocks: %lu (%lu bytes),\nEvent data buffer size: %lu\n", tempHd.nbOfBlocks, tempHd.eventsBlockBufSize);
   amTrace("data buffer size: %lu\n", tempHd.eventDataBufSize);

   amTrace("td: %u ", tempHd.division);
   amTrace("packed: %s ", tempHd.bPacked?"YES":"NO");

   lzo_voidp pPackedEvents=0;
   lzo_voidp pPackedData=0;
   lzo_uint newEventSize=0;
   lzo_uint newDataSize=0;


   if( tempHd.nbOfBlocks==0 || tempHd.eventsBlockBufSize==0 || tempHd.eventDataBufSize==0 ){

    #ifndef SUPRESS_CON_OUTPUT
        printf("Error: File %s has no data or event blocks!\n",pFilePath);
    #endif

    amTrace("Error: File %s has no data or event blocks!\n",pFilePath);

    #ifdef ENABLE_GEMDOS_IO
        amTrace("[GEMDOS] Closing file handle : [%d] \n", fh);
        S16 err=Fclose(fh);

        if(err!=GDOS_OK){
          amTrace("[GEMDOS] Error closing file handle : [%d] %s\n", fh, getGemdosError(err));
        }

    #else
        fclose(fp); fp=0;
    #endif


    amFree(pNewSeq);
    return NULL;

   }else{
       // update info from header
        pNewSeq->nbOfBlocks = tempHd.nbOfBlocks;
        pNewSeq->eventsBlockBufferSize=tempHd.eventsBlockBufSize;
        pNewSeq->dataBufferSize = tempHd.eventDataBufSize;
        pNewSeq->timeDivision = tempHd.division;
        pNewSeq->version=tempHd.version;
        pNewSeq->timeDivision=tempHd.division;
        pNewSeq->bPacked=tempHd.bPacked;

        // ok
        amTrace("Blocks in sequence: %lu \n", pNewSeq->nbOfBlocks);
        amTrace("Event data buffer size: %lu \n", pNewSeq->dataBufferSize);
        amTrace("Events block size: %lu \n", pNewSeq->eventsBlockBufferSize);
        amTrace("Packed: %s \n", pNewSeq->bPacked?"YES":"NO");

        if(pNewSeq->bPacked!=FALSE){


            lzo_voidp pPackedEventsSource=0;


            if(lzo_init()!=LZO_E_OK){
              amTrace("Error: Could't initialise LZO library. Cannot depack data. \n");
              return NULL;
            }

                // unpack them to temp buffers

                amTrace("[LZO] Allocating temp events buffer\n");
                U32 amount = pNewSeq->eventsBlockBufferSize*4;

                pPackedEvents=(lzo_voidp)amMallocEx(amount,PREFER_TT);
                pPackedEventsSource=(lzo_voidp)amMallocEx(pNewSeq->eventsBlockBufferSize,PREFER_TT);

                if(pPackedEvents!=NULL&&pPackedEventsSource!=NULL){

                    amMemSet(pPackedEvents,0,amount);
                    amMemSet(pPackedEventsSource,0,pNewSeq->eventsBlockBufferSize);

                    // fill buffer with packed data

#ifdef ENABLE_GEMDOS_IO
                    S32 read = Fread(fh, pNewSeq->eventsBlockBufferSize, pPackedEventsSource);
                    amTrace("[GEMDOS] Read events buffer data %lu \n",read);
#else

#error TODO

#endif
                    amTrace("[LZO] Decompressing events block...\n");
                    int decResult=lzo1x_decompress(pPackedEventsSource,read,pPackedEvents,&newEventSize,LZO1X_MEM_DECOMPRESS);

                    if( decResult == LZO_E_OK){
                        amTrace("[LZO] Decompressed events buffer %ld->%ld bytes\n",pNewSeq->eventsBlockBufferSize,newEventSize);
                        pNewSeq->eventsBlockBufferSize=newEventSize;

                        //create linear buffer
                        //reeserve mem

                        //TODO: copy depacked data
                        amMemCpy(pNewSeq->eventBlocksPtr,pPackedEvents,newEventSize);

                    }else{
                        amTrace("[LZO] Error: Events block decompression error: %d at %ld\n",decResult,newEventSize);
                    }



                    amFree(pPackedEvents);
                }

                amTrace("[LZO] Allocating temp data buffer\n");
                amount = pNewSeq->dataBufferSize+(pNewSeq->dataBufferSize/16)+64+3;
                pPackedData=(lzo_voidp)amMallocEx(amount,PREFER_TT);

                if(pPackedData!=NULL){
                    amMemSet(pPackedData,0,amount);

#ifdef ENABLE_GEMDOS_IO
                    amTrace("[GEMDOS] Read events data buffer\n");
                    S32 read=Fread(fh,pNewSeq->dataBufferSize,pPackedData);
#else
#error TODO

#endif
                    amTrace("[LZO] Decompressing data block...\n");
                    int decResult = lzo1x_decompress(pNewSeq->eventDataPtr,read,pPackedData,&newDataSize,LZO1X_MEM_DECOMPRESS);

                    if( decResult == LZO_E_OK){
                        amTrace("[LZO] Decompressed events buffer %ld->%ld bytes\n",pNewSeq->dataBufferSize,newDataSize);

                        pNewSeq->dataBufferSize=newDataSize;


                    }else{
                         amTrace("[LZO] Error: Data block decompression error: %d at %ld\n",decResult,newDataSize);
                    }


                    amFree(pPackedData);
                }

        }

        U32 lbAllocAdr=0;

        // create linear buffer
        if(createLinearBuffer(&(pNewSeq->lbEventsBuffer),pNewSeq->eventsBlockBufferSize+255,PREFER_TT)<0){

            #ifndef SUPRESS_CON_OUTPUT
            printf("Error: loadSequence() Couldn't allocate memory for event block buffer.\n");
            #endif

            amTrace("Error: loadSequence() Couldn't allocate memory for event block buffer.\n");

            #ifdef ENABLE_GEMDOS_IO
                amTrace("[GEMDOS] Closing file handle : [%d] \n", fh);

                S16 err=Fclose(fh);

                if(err!=GDOS_OK){
                  amTrace("[GEMDOS] Error closing file handle : [%d] \n", fh, getGemdosError(err));
                }

            #else
                fclose(fp); fp=0;
            #endif

            amFree(pNewSeq);
            return NULL;
         }

         // allocate contigous / linear memory for pNewSeq->NbOfBlocks events
         lbAllocAdr=(U32)linearBufferAlloc(&(pNewSeq->lbEventsBuffer), (pNewSeq->eventsBlockBufferSize)+255);
         lbAllocAdr+=255;
         lbAllocAdr&=0xfffffff0;

         pNewSeq->eventBlocksPtr = (U8 *)lbAllocAdr;

         if(pNewSeq->eventBlocksPtr==0){

#ifndef SUPRESS_CON_OUTPUT
            printf("Error: loadSequence() Linear buffer out of memory.\n");
#endif
            amTrace("Error: loadSequence() Linear buffer out of memory.\n");

            #ifdef ENABLE_GEMDOS_IO
                amTrace("[GEMDOS] Closing file handle : [%d] \n", fh);

                S16 err=Fclose(fh);

                if(err!=GDOS_OK){
                  amTrace("[GEMDOS] Error closing file handle : [%d] \n", fh, getGemdosError(err));
                }

            #else
                fclose(fp); fp=0;
            #endif

             amFree(pNewSeq);
             return NULL;
         }

         amTrace("Allocated %lu kb for event block buffer\n",(pNewSeq->eventsBlockBufferSize)/1024);

         if(createLinearBuffer(&(pNewSeq->lbDataBuffer),pNewSeq->dataBufferSize+255,PREFER_TT)<0){

            #ifndef SUPRESS_CON_OUTPUT
                printf("Error: loadSequence() Couldn't allocate memory for temp data buffer. \n");
            #endif

             amTrace("Error: loadSequence() Couldn't allocate memory for temp data buffer. \n");

            #ifdef ENABLE_GEMDOS_IO
                amTrace("[GEMDOS] Closing file handle : [%d] \n", fh);

                S16 err=Fclose(fh);

                if(err!=GDOS_OK){
                  amTrace("[GEMDOS] Error closing file handle : [%d] \n", fh, getGemdosError(err));
                }

            #else
                fclose(fp); fp=0;
            #endif

             // destroy block buffer
             destroyLinearBuffer(&(pNewSeq->lbEventsBuffer));
             amFree(pNewSeq);

             return NULL;
         }

       // alloc memory for data buffer from linear allocator
       lbAllocAdr=(U32)linearBufferAlloc(&(pNewSeq->lbDataBuffer), pNewSeq->dataBufferSize+255);
       lbAllocAdr+=255;
       lbAllocAdr&=0xfffffff0;


       pNewSeq->eventDataPtr = (U8*)lbAllocAdr;
       amTrace("Allocated %lu kb for event data buffer\n",(pNewSeq->dataBufferSize)/1024);

       if(pNewSeq->eventDataPtr==0){

           #ifndef SUPRESS_CON_OUTPUT
                printf("Error: loadSequence() Linear buffer out of memory.\n");
           #endif

           amTrace("Error: loadSequence() Linear buffer out of memory.\n");

           // destroy block buffer
           destroyLinearBuffer(&(pNewSeq->lbEventsBuffer));
           destroyLinearBuffer(&(pNewSeq->lbDataBuffer));

           #ifdef ENABLE_GEMDOS_IO
            amTrace("[GEMDOS] Closing file handle : [%d] \n", fh);

            S16 err=Fclose(fh);

            if(err!=GDOS_OK){
                amTrace("[GEMDOS] Error closing file handle : [%d] \n", fh, getGemdosError(err));
            }

           #else
            fclose(fp); fp=0;
           #endif

           amFree(pNewSeq);
           return NULL;
       }

     }

   // read raw data if file isn't packed
   if(pNewSeq->bPacked!=FALSE){
        // copy depacked data to buffers
        amMemCpy(pNewSeq->eventBlocksPtr,pPackedEvents,newEventSize);
        amMemCpy(pNewSeq->dataBufferSize,pPackedEvents,newEventSize);

        //free buffers

   }

   // read raw data if file isn't packed
   if(pNewSeq->bPacked==FALSE){
     // load decompressed data directly from file
     // load event block
        tMEMSIZE amount = pNewSeq->eventsBlockBufferSize;

#ifdef ENABLE_GEMDOS_IO
         read=Fread(fh,amount,(void *)pNewSeq->eventBlocksPtr);

         if(read<amount){
             amTrace("[GEMDOS] error, read bytes: %ld, expected read: %ld ",read,amount);
             // todo cleanup
             return NULL;
         }
#else
        fread((void *)pNewSeq->eventBlocksPtr,amount,1,fp);
#endif

        amount = pNewSeq->dataBufferSize;

        // load data block
#ifdef ENABLE_GEMDOS_IO

        read=Fread(fh,amount,(void *)pNewSeq->eventDataPtr);

        if(read<amount){
            amTrace("[GEMDOS] error, read bytes: %ld, expected read: %ld ",read,amount);
            // todo cleanup
            return NULL;
        }
#else
        fread((void *)pNewSeq->eventDataPtr,amount,1,fp);
#endif
 }

#ifdef LOAD_TEST
    amTrace("[LOAD TEST]\n");
    U32 blockNb=0;
    U8 count=0;

for (int i=0;i<255;++i){
    amTrace("[0x%x]",pNewSeq->eventBlocksPtr[i]);

}
amTrace("\n");

while(blockNb<pNewSeq->nbOfBlocks){

      U32 addr=((U32)pNewSeq->eventBlocksPtr)+pNewSeq->eventsBlockOffset;

      U8 *pEventPtr=(U8 *)(addr);
      U32 d=readVLQ(pEventPtr,&count);

      pEventPtr+=count;

      sNktBlock_t *eBlk=(sNktBlock_t *)(pEventPtr);

      amTrace("delta [%lu] type:[%u] size:[%u] bytes offset: [%lu] \n",d, eBlk->msgType, eBlk->blockSize,eBlk->bufferOffset);

      if(eBlk->blockSize>0){
          amTrace("[DATA] ");

          U8 *data = (U8 *)((U32)(pNewSeq->eventDataPtr)+eBlk->bufferOffset);

          for(int j=0;j<eBlk->blockSize;++j){
           amTrace("0x%02x ",data[j]);
          }

          amTrace(" [/DATA]\n");

      }
      pNewSeq->eventsBlockOffset+=count;
      pNewSeq->eventsBlockOffset+=sizeof(sNktBlock_t);


      ++blockNb;
    }

#endif

// close file
#ifdef ENABLE_GEMDOS_IO

    amTrace("[GEMDOS] Closing file handle : [%d] \n", fh);
    S16 err=Fclose(fh);

    if(err!=GDOS_OK){
        amTrace("[GEMDOS] Error closing file handle : [%d] \n", fh, getGemdosError(err));
    }

#else
    fclose(fp);fp=0;
#endif

 return pNewSeq;
}

void destroySequence(sNktSeq *pSeq){

    if(pSeq==0) return;

    if(pSeq->nbOfBlocks==0){

        amMemSet(pSeq,0,sizeof(sNktSeq));
        amFree(pSeq);
        return;

    }else{

        // release linear buffer
        linearBufferFree(&(pSeq->lbEventsBuffer));
        linearBufferFree(&(pSeq->lbDataBuffer));

        //clear struct
        amMemSet(pSeq,0,sizeof(sNktSeq));
        amFree(pSeq);
        return;
    }
}


////////////////////////////////////////////////// replay control

BOOL isSequencePlaying(void){

 if(g_CurrentNktSequence!=0){
     U16 state=g_CurrentNktSequence->sequenceState;
     if((state&NKT_PS_PLAYING)&&(!(state&NKT_PS_PAUSED)))
        return TRUE;
       else
        return FALSE;
 }

 return FALSE;
}


void stopSequence(void){
 if(g_CurrentNktSequence!=0){
  U16 state=g_CurrentNktSequence->sequenceState;
  if((state&NKT_PS_PLAYING)||(state&NKT_PS_PAUSED)){
       g_CurrentNktSequence->sequenceState&=(~(NKT_PS_PLAYING|NKT_PS_PAUSED));

#ifndef SUPRESS_CON_OUTPUT
       printf("Stop sequence\n");
#endif

  }

    //all notes off
    am_allNotesOff(16);

    #ifdef IKBD_MIDI_SEND_DIRECT
      Supexec(flushMidiSendBuffer);
    #endif
  }
}

void pauseSequence(){

     if(g_CurrentNktSequence!=0){
        U16 state=g_CurrentNktSequence->sequenceState;

          if((state&NKT_PS_PLAYING)&&(!(state&NKT_PS_PAUSED))){
           g_CurrentNktSequence->sequenceState&=(~NKT_PS_PLAYING);
           g_CurrentNktSequence->sequenceState|=NKT_PS_PAUSED;

           // all notes off
           am_allNotesOff(16);

#ifdef IKBD_MIDI_SEND_DIRECT
           Supexec(flushMidiSendBuffer);
#endif

#ifndef SUPRESS_CON_OUTPUT
           printf("Pause sequence\n");
#endif

           return;
          }else if(!(state&NKT_PS_PLAYING)&&(state&NKT_PS_PAUSED) ){
            g_CurrentNktSequence->sequenceState&=(~NKT_PS_PAUSED); //unpause
            g_CurrentNktSequence->sequenceState|=NKT_PS_PLAYING;  //set playing state
          }
      }
 } //pauseSequence

// play sequence
void playSequence(void){

if(g_CurrentNktSequence!=0){
  U16 state=g_CurrentNktSequence->sequenceState;

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

void NktInit(const eMidiDeviceType devType, const U8 channel){

    initDebug("NKTLOG.LOG");

    // now depending on the connected device type and chosen operation mode
    // set appropriate channel
    // prepare device for receiving messages

     setupMidiDevice(devType,channel);
     currentMasterVolume=64;
     requestedMasterVolume=64;

    //

}


void NktDeinit(){
#ifdef IKBD_MIDI_SEND_DIRECT
    // send content of midi buffer to device
    Supexec(flushMidiSendBuffer);
#endif

    deinitDebug();
}

#ifdef DEBUG_BUILD

// debug stuff
static const U8 *getSequenceStateStr(const U16 state){

 if( !(state&NKT_PS_PLAYING) && (state&NKT_PS_PAUSED) ){
    return "Paused";
 }else if(state&NKT_PS_PLAYING && (!(state&NKT_PS_PAUSED))){
    return "Playing";
 }else if(!(state&NKT_PS_PLAYING)){
    return "Stopped...";
 }
}


void printNktSequenceState(){

#ifndef SUPRESS_CON_OUTPUT

if(g_CurrentNktSequence){
    printf("Td/PPQN: %u\n",g_CurrentNktSequence->timeDivision);
    printf("Time step: %lu\n",g_CurrentNktSequence->timeStep);
    printf("Time elapsedFrac: %lu\n",g_CurrentNktSequence->timeElapsedFrac);
    printf("\tTime elapsed: %lu\n",g_CurrentNktSequence->timeElapsedInt);
    printf("\tDefault Tempo: %lu\n",g_CurrentNktSequence->defaultTempo.tempo);
    printf("\tLast Tempo: %lu\n",g_CurrentNktSequence->currentTempo.tempo);
    printf("\tSequence state: 0x%x\n",getSequenceStateStr(g_CurrentNktSequence->sequenceState));
  }

 printMidiSendBufferState();

#endif

}

static const U8* _arNktEventName[NKT_MAX_EVENT]={
    "NKT_MIDIDATA",
    "NKT_TEMPO_CHANGE",
    "NKT_JUMP",
    "NKT_TRIGGER",
    "NKT_END"
};

const U8 *getEventTypeName(U16 type){
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

void setMidiMasterVolume(U8 vol){
    if(vol<=127){
        requestedMasterVolume=vol;
    }
}

U8 getMidiMasterVolume(){;
    return currentMasterVolume;
}


U32 saveEventDataBlocks(S16 fh,sNktSeq *pSeq){
 S32 written=0;
        // save event block
        amTrace("[MID2NKT] Saving event block.[%ld bytes] \n",pSeq->eventsBlockBufferSize);

        written = Fwrite(fh,pSeq->eventsBlockBufferSize,(void *)pSeq->eventBlocksPtr);

        if(written<pSeq->eventsBlockBufferSize){
           amTrace("[GEMDOS]Fatal error: Events block write error, written: %ld , expected %ld bytes\n", written, pSeq->eventsBlockBufferSize);
           amTrace("[GEMDOS] Error: %s\n", getGemdosError((S16)written));

           return -1;
        }else{
            amTrace("[GEMDOS] written: %ld bytes\n", written);

        }

        // save data block
        amTrace("[MID2NKT] Saving data block.[%ld bytes] \n",pSeq->dataBufferSize);

        written=Fwrite(fh,pSeq->dataBufferSize,(void *)pSeq->eventDataPtr);

        if(written<pSeq->dataBufferSize)
        {
           amTrace("[GEMDOS]Fatal error: Event data block write error, written: %ld , expected %ld bytes\n", written, pSeq->dataBufferSize);
           amTrace("[GEMDOS] Error: %s\n", getGemdosError((S16)written));

           return -1;
        }else{
            amTrace("[GEMDOS] written: %ld bytes\n", written);
        }

    return 0;
}



U32 saveSequence(sNktSeq *pSeq,const U8 *filepath,BOOL bCompress){


if(filepath==0||strlen(filepath)==0) {
    amTrace("[MID2NKT] Fatal error, path is empty.\n");

}

// create header
sNktHd nktHd;

// set header
amTrace("[MID2NKT] Init header...\n");

setNktHeader(&nktHd, pSeq, bCompress);

#ifdef ENABLE_GEMDOS_IO

 S16 fh=GDOS_INVALID_HANDLE;

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

          amTrace("[LZO] init ok.\n");

          // allocate work buffers
          amTrace("[LZO] Allocating work buffer.\n");

          lzo_voidp workMem=(lzo_voidp)amMallocEx(LZO1X_1_MEM_COMPRESS,PREFER_TT); // lzo work buffer

          if(workMem!=0){

              amMemSet(workMem,0,LZO1X_1_MEM_COMPRESS);

              amTrace("[LZO] Compressing events block.\n");
              tMEMSIZE tempBufSize=pSeq->eventsBlockBufferSize+pSeq->eventsBlockBufferSize/16;
              lzo_bytep tempBuffer=(lzo_bytep)amMallocEx(tempBufSize,PREFER_TT);

              if(tempBuffer==NULL){
                  amTrace("[LZO] Error, no memory for events block output buffer.\n");

                  // free work mem
                  amFree(workMem);
                  return -1;
              }else{
                  amMemSet(tempBuffer,0,tempBufSize);
              }

              // compress
              lzo_uint nbBytesPacked=0;
              if(lzo1x_1_compress(pSeq->eventBlocksPtr,pSeq->eventsBlockBufferSize,tempBuffer,&nbBytesPacked,workMem)==LZO_E_OK){
                    amTrace("[LZO] Event data compressed %lu->%lu bytes.\n",pSeq->eventsBlockBufferSize,nbBytesPacked);

                    /* check for an incompressible block */
                    if (nbBytesPacked >= pSeq->eventsBlockBufferSize){
                            amTrace("[LZO] Error: Event block contains incompressible data.\n");
                        return -1;
                    }

                    //copy output buffer with packed data
                    //TODO: shrink existing buffer somehow

                    pSeq->eventsBlockBufferSize=nbBytesPacked;
                    nktHd.eventsBlockBufSize=nbBytesPacked;
                    amMemCpy(pSeq->eventBlocksPtr,tempBuffer,nbBytesPacked);

              }else{
                 amTrace("[LZO] Error: Compression failed.\n");
              }

              amFree(tempBuffer);

              amTrace("[LZO] Compressing data block.\n");
              tempBufSize=pSeq->dataBufferSize+pSeq->dataBufferSize/16;
              tempBuffer=(lzo_bytep)amMallocEx(tempBufSize,PREFER_TT);

              if(tempBuffer==NULL){
                  amTrace("[LZO] Error, no memory for data block output buffer.\n");

                  // free work mem
                  amFree(workMem);
                  return -1;
              }else{
                  amMemSet(tempBuffer,0,tempBufSize);
              }

              // compress data block
              nbBytesPacked=0;
              if(lzo1x_1_compress(pSeq->eventDataPtr,pSeq->dataBufferSize,tempBuffer,&nbBytesPacked,workMem)==LZO_E_OK){
                    amTrace("[LZO] Data block compressed %lu->%lu bytes.\n",pSeq->eventsBlockBufferSize,nbBytesPacked);

                    /* check for an incompressible block */
                      if (nbBytesPacked >= pSeq->dataBufferSize){
                            amTrace("[LZO] Error: Data block contains incompressible data.\n");
                        return -1;
                      }

                    //copy output buffer with packed data
                    //TODO: shrink existing buffer somehow
                    pSeq->dataBufferSize=nbBytesPacked;
                    nktHd.eventDataBufSize=nbBytesPacked;
                    amMemCpy(pSeq->eventDataPtr,tempBuffer,nbBytesPacked);

              }else{
                 amTrace("[LZO] Error: Compression failed.\n");
              }

              pSeq->bPacked=TRUE;

              //copy output buffer with packed data to
              amFree(tempBuffer);

              // free work mem
              amFree(workMem);

          }else{
              amTrace("[LZO] Error couldn't allocate compression work memory.\n");
              return -1;
          }

          // save header
          S32 written=0;

          // save header
          amTrace("[MID2NKT] Saving header... \n");
          written=Fwrite(fh, sizeof(sNktHd), &nktHd);

          if(written<sizeof(sNktHd)){
             amTrace("[GEMDOS]Fatal error: Header write error, written: %ld, expected %ld\n", written, sizeof(sNktHd));
             amTrace("[GEMDOS] Error: %s\n", getGemdosError((S16)written));
             return -1;
          }else{
              amTrace("[GEMDOS] written: %ld bytes\n", written);
          }

          // write data / event blocks
          if(saveEventDataBlocks(fh,pSeq)<0){
              return -1;
          }

         return 0;
     }else{

         amTrace("[GEMDOS] Created file handle: %d\n",fh);

         S32 written=0;

         // save header
         amTrace("[MID2NKT] Saving header... \n");
         written=Fwrite(fh, sizeof(sNktHd), &nktHd);

         if(written<sizeof(sNktHd)){
            amTrace("[GEMDOS]Fatal error: Header write error, written: %ld, expected %ld\n", written, sizeof(sNktHd));
            amTrace("[GEMDOS] Error: %s\n", getGemdosError((S16)written));
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
     S16 err = Fclose(fh);

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

 // save event block
 // save data block

 // close file

#endif

 return 0;
}


void setNktHeader(sNktHd* header, const sNktSeq *pNktSeq, const BOOL bCompress){

    if(header){
        // clear header
        amMemSet(header, 0L, sizeof(sNktHd));
        header->id=ID_NKT;
        header->bPacked = bCompress;
        header->nbOfBlocks = pNktSeq->nbOfBlocks;
        header->eventDataBufSize = pNktSeq->dataBufferSize;
        header->eventsBlockBufSize = pNktSeq->eventsBlockBufferSize;
        header->division = pNktSeq->timeDivision;
        header->version = NKT_VERSION;

        amTrace("Set header: event data buffer: %ld events block buffer: %ld\n", header->eventDataBufSize,header->eventsBlockBufSize);


    }
}

