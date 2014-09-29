
/**  Copyright 2007-2014 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#include "nkt.h"
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
  g_CurrentNktSequence->currentBlockId=0;

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
                  amTrace("Update 25hz: %ld  [0xlx]\n",pSeq->currentTempo.tuTable[i],pSeq->currentTempo.tuTable[i]);
              } break;
              case NKT_U50HZ:{
                  if(tempPPU<65536){
                        pSeq->defaultTempo.tuTable[i]=pSeq->currentTempo.tuTable[i]=((tempPPU*65536)/60)/50;
                  }else{
                        pSeq->defaultTempo.tuTable[i]=pSeq->currentTempo.tuTable[i]=((tempPPU/60)*65536)/50;
                  }
                   amTrace("Update 50hz: %ld [0xlx]\n",pSeq->currentTempo.tuTable[i],pSeq->currentTempo.tuTable[i]);
              } break;
              case NKT_U100HZ:{
                  if(tempPPU<65536){
                       pSeq->defaultTempo.tuTable[i]=pSeq->currentTempo.tuTable[i]=((tempPPU*65536)/60)/100;
                  }else{
                       pSeq->defaultTempo.tuTable[i]=pSeq->currentTempo.tuTable[i]=((tempPPU/60)*65536)/100;
                  }
                   amTrace("Update 100hz: %ld [0xlx]\n",pSeq->currentTempo.tuTable[i],pSeq->currentTempo.tuTable[i]);
              } break;
              case NKT_U200HZ:{
                  if(tempPPU<65536){
                       pSeq->defaultTempo.tuTable[i]=pSeq->currentTempo.tuTable[i]=((tempPPU*65536)/60)/200;
                  }else{
                       pSeq->defaultTempo.tuTable[i]=pSeq->currentTempo.tuTable[i]=((tempPPU/60)*65536)/200;
                  }
                   amTrace("Update 200hz: %ld [0xlx]\n",pSeq->currentTempo.tuTable[i],pSeq->currentTempo.tuTable[i]);
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

      nktBlk=(sNktBlock_t *)&(g_CurrentNktSequence->pEvents[g_CurrentNktSequence->currentBlockId]);

      // track end?
      if(nktBlk->msgType&NKT_END||g_CurrentNktSequence->currentBlockId>=g_CurrentNktSequence->NbOfBlocks){
         onEndSequence();
         return;
     }

     // update
     g_CurrentNktSequence->timeElapsedFrac += g_CurrentNktSequence->timeStep;
     TimeAdd = g_CurrentNktSequence->timeElapsedFrac >> 16;
     g_CurrentNktSequence->timeElapsedFrac &= 0xffff;

     // timestep forward
     g_CurrentNktSequence->timeElapsedInt=g_CurrentNktSequence->timeElapsedInt+TimeAdd;

     if( g_CurrentNktSequence->timeElapsedInt==nktBlk->delta||nktBlk->delta==0){
         g_CurrentNktSequence->timeElapsedInt -= nktBlk->delta;

         // tempo change ?
         if(nktBlk->msgType&NKT_TEMPO_CHANGE){
            // set new tempo
            U32 *pData = (U32 *)nktBlk->pData;
            g_CurrentNktSequence->currentTempo.tempo=*pData;
            pData++;

            // get precalculated timestep
            g_CurrentNktSequence->timeStep=pData[g_CurrentNktSequence->currentUpdateFreq];

            //next event
            ++(g_CurrentNktSequence->currentBlockId);
            nktBlk=(sNktBlock_t *)&(g_CurrentNktSequence->pEvents[g_CurrentNktSequence->currentBlockId]);
        }

  #ifdef IKBD_MIDI_SEND_DIRECT
          amMemCpy(MIDIsendBuffer,nktBlk->pData, nktBlk->blockSize);
          MIDIbytesToSend=nktBlk->blockSize;
  #else
          //send to xbios
          amMidiSendData(nktBlk->blockSize,nktBlk->pData);
  #endif

          //go to next event
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
    sNktSeq *pNewSeq=amMallocEx(sizeof(sNktSeq),PREFER_TT);

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
    S16 fh=GDOS_OK;
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

   pNewSeq->NbOfBlocks=tempHd.NbOfBlocks;
   pNewSeq->dataBufferSize=tempHd.NbOfBytesData;
   pNewSeq->timeDivision=tempHd.division;

   if(pNewSeq->NbOfBlocks==0 || pNewSeq->dataBufferSize==0){

    #ifndef SUPRESS_CON_OUTPUT
        printf("Error: File %s has no data or event blocks!\n",pFilePath);
    #endif

    amTrace("Error: File %s has no data or event blocks!\n",pFilePath);

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

   }else{
        amTrace("Blocks in sequence: %lu\n",pNewSeq->NbOfBlocks);
        amTrace("Data in bytes: %lu\n",pNewSeq->dataBufferSize);

        // allocate contigous/linear memory for pNewSeq->NbOfBlocks events
        if(createLinearBuffer(&(pNewSeq->eventBuffer),pNewSeq->NbOfBlocks*sizeof(sNktBlock_t),PREFER_TT)<0){

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

         // alloc memory for event blocks from linear buffer
         pNewSeq->pEvents=(sNktBlock_t *)linearBufferAlloc(&(pNewSeq->eventBuffer), pNewSeq->NbOfBlocks*sizeof(sNktBlock_t));

         if(pNewSeq->pEvents==0){

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

         amTrace("Allocated %lu kb for event block buffer\n",(pNewSeq->NbOfBlocks*sizeof(sNktBlock_t))/1024);

         if(createLinearBuffer(&(pNewSeq->dataBuffer),pNewSeq->dataBufferSize,PREFER_TT)<0){

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
             destroyLinearBuffer(&(pNewSeq->eventBuffer));
             amFree(pNewSeq);

             return NULL;
         }

       // alloc memory for data buffer from linear allocator
       pNewSeq->pEventDataBuffer = linearBufferAlloc(&(pNewSeq->dataBuffer), pNewSeq->dataBufferSize);

       amTrace("Allocated %lu kb for event data buffer\n",(pNewSeq->dataBufferSize)/1024);

       if(pNewSeq->pEventDataBuffer==0){

           #ifndef SUPRESS_CON_OUTPUT
                printf("Error: loadSequence() Linear buffer out of memory.\n");
           #endif

           amTrace("Error: loadSequence() Linear buffer out of memory.\n");

           // destroy block buffer
           destroyLinearBuffer(&(pNewSeq->eventBuffer));
           destroyLinearBuffer(&(pNewSeq->dataBuffer));

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

    // process blocks
    sNktBlk blk;
    BOOL bFinished=FALSE;
    int i=0;
    U8 *pTempPtr = pNewSeq->pEventDataBuffer;

    // check if file is compressed
    if(tempHd.bPacked!=FALSE){

        #ifdef ENABLE_GEMDOS_IO
            Fseek(0,fh,SEEK_SET);
            Fseek(sizeof(sNktHd),fh,SEEK_SET);
        #else
            fseek(fp,0,SEEK_SET);
            fseek(fp,sizeof(sNktHd),SEEK_SET);
        #endif

        // allocate temp buffer for unpacked data
        U32 destSize=tempHd.NbOfBlocks * sizeof(sNktBlock_t) + tempHd.NbOfBytesData;

        #ifndef SUPRESS_CON_OUTPUT
            printf("[LZO] packed: %lu, unpacked: %lu\n", tempHd.bytesPacked, destSize);
        #endif

        void *pDepackBuf = amMallocEx(tempHd.bytesPacked,PREFER_TT); //packed data buffer
        void *pOutputBuf = amMallocEx(destSize,PREFER_TT);    // depacked midi data
        void *pWrkBuffer = amMallocEx(destSize/16,PREFER_TT); // lzo work buffer

        if(pDepackBuf!=0&&pOutputBuf!=0&&pWrkBuffer!=0){
         amMemSet(pDepackBuf,0,tempHd.bytesPacked);
         amMemSet(pOutputBuf,0,destSize);
         amMemSet(pWrkBuffer,0,destSize/16);

#ifdef ENABLE_GEMDOS_IO
         if(Fread(fh,tempHd.bytesPacked,pDepackBuf)==tempHd.bytesPacked)
#else
         if(fread(pDepackBuf,1,tempHd.bytesPacked,fp)==tempHd.bytesPacked)
#endif
         {

             // decompress data
#ifndef SUPRESS_CON_OUTPUT
             printf("[LZO] Decompressing ...\n");
#endif
                 if(lzo1x_decompress_safe(pDepackBuf,tempHd.bytesPacked,pOutputBuf,&destSize,pWrkBuffer)==LZO_E_OK){
                     // process data

#ifndef SUPRESS_CON_OUTPUT
                     printf("[LZO] Block decompressed: %lu, packed: %lu\n",destSize,tempHd.bytesPacked);
#endif

                     U8 *pData = (U8 *)pOutputBuf;

                     // process decompressed data directly from file
                     while(pData!=(pData+destSize)&&i<pNewSeq->NbOfBlocks){
                             U32 delta=0;
                             U8 count=0;

                             delta=readVLQ(pData,&count);
                             amTrace("[R] delta:[%lu] byte count:[%d]\t",delta, count);
                             pData+=count;

                             //  read msg block
                             //  fread(&blk,sizeof(sNktBlk),1,fp);
                             sNktBlk *pTemp=(sNktBlk *)pData;
                             pNewSeq->pEvents[i].delta=delta;
                             pNewSeq->pEvents[i].msgType=pTemp->msgType;
                             pNewSeq->pEvents[i].blockSize=pTemp->blockSize;

                             pData+=sizeof(sNktBlk);
                             amTrace("[RB] delta: [%lu] type:[%d] block size:[%u] bytes\n", delta, pTemp->msgType, pTemp->blockSize );

                             if(pNewSeq->pEvents[i].blockSize!=0){
                                 // assign buffer memory pointer for data, size blk.blockSize
                                 pNewSeq->pEvents[i].pData=pTempPtr;
                                 pTempPtr+=pTemp->blockSize;

                                 amMemCpy(pNewSeq->pEvents[i].pData,pData,pTemp->blockSize);
                                 pData+=pTemp->blockSize;

                                 if(pTemp->msgType&NKT_TEMPO_CHANGE){
                                    U32 *pTempo=(U32 *)pNewSeq->pEvents[i].pData;
                                    amTrace("[Tempo block] tempo %lu, blocksize: %d\n",(U32)(*pTempo),blk.blockSize);
                                 }
                             }
                         ++i;
                     }
#ifndef SUPRESS_CON_OUTPUT
                     printf("[LZO] OK\n");
#endif

                 }else{
#ifndef SUPRESS_CON_OUTPUT
                     printf("[LZO] Data decompression error.\n");
#endif
                 }
            }else{
#ifndef SUPRESS_CON_OUTPUT
             printf("[LZO] Data read failed ..\n");
#endif
          }

        // deallocate temp buffers
            amFree(pWrkBuffer);
            amFree(pOutputBuf);
            amFree(pDepackBuf);

        }else{
			amFree(pWrkBuffer);
            amFree(pOutputBuf);
            amFree(pDepackBuf);

#ifndef SUPRESS_CON_OUTPUT
            printf("[LZO] Error: Couldn't allocate work buffers.\n");
            getchar();
#endif

#ifdef ENABLE_GEMDOS_IO
            S16 err=Fclose(fh);

            if(err!=GDOS_OK){
              amTrace("[GEMDOS] Error closing file handle : [%d] \n", fh, getGemdosError(err));
            }

#else
            fclose(fp);fp=0;
#endif

            return NULL;
        }
    }else{
        // process decompressed data directly from file

#ifdef ENABLE_GEMDOS_IO
//TODO
#warning TODO add end of file check
        while((i<pNewSeq->NbOfBlocks)){

#else

        while( (!feof(fp))&&(i<pNewSeq->NbOfBlocks)){

#endif

                U32 tempDelta,delta=0;
                U8 count=0;
#ifdef ENABLE_GEMDOS_IO
                S32 read=Fread(fh,sizeof(U32),&tempDelta);

                if(read<0){
                      //GEMDOS ERROR TODO, display error for now
                      amTrace("[GEMDOS] Error: %s\n",getGemdosError(read));
                }else{
                    if(read!=sizeof(U32)){
                        amTrace("[GEMDOS] Read error. Unexpected EOF. expected: %d, read: %d\n",sizeof(U32),read);
                    }
                }
#else
                fread(&tempDelta,sizeof(U32),1,fp);
#endif

                delta=readVLQ((U8*)&tempDelta,&count);
                amTrace("[R] delta:[%lu], byte count:[%d]\t", count,delta );

#ifdef ENABLE_GEMDOS_IO
                // rewind depending how many bytes were read from VLQ (size of former read - count of bytes read)
                Fseek(-(sizeof(U32)-count),fh,SEEK_CUR);

                // read msg block
                read=Fread(fh, sizeof(sNktBlk), &blk);

                if(read<0){
                  //GEMDOS ERROR TODO, display error for now
                  amTrace("[GEMDOS] Error: %s\n",getGemdosError(read));
                }else if(read!=sizeof(sNktBlk)){
                  amTrace("[GEMDOS] Read error, expected: %d, read: %d\n",sizeof(sNktBlk),read);
                }

#else
                // rewind depending how many bytes were read from VLQ (size of former read - count of bytes read)
                fseek(fp,-(sizeof(U32)-count),SEEK_CUR);

                // read msg block
                fread(&blk,sizeof(sNktBlk),1,fp);
#endif


                pNewSeq->pEvents[i].delta=delta;
                pNewSeq->pEvents[i].msgType=blk.msgType;
                pNewSeq->pEvents[i].blockSize=blk.blockSize;

                amTrace("[RB] delta [%lu] type:[%d] size:[%u] bytes\n", delta, blk.msgType, blk.blockSize );

                if(pNewSeq->pEvents[i].blockSize!=0){

                    // assign buffer memory pointer for data, size blk.blockSize
                    pNewSeq->pEvents[i].pData=pTempPtr;
                    pTempPtr+=blk.blockSize;
#ifdef ENABLE_GEMDOS_IO
                    read=Fread(fh,blk.blockSize,pNewSeq->pEvents[i].pData);

                    if(read!=blk.blockSize){
                        amTrace("[GEMDOS] Read error, expected: %d, read: %d\n",blk.blockSize,read);
                    }
#else
                    fread(pNewSeq->pEvents[i].pData,blk.blockSize,1,fp);
#endif

                    if(blk.msgType==NKT_TEMPO_CHANGE){
                           U32 *pTempo=(U32 *)pNewSeq->pEvents[i].pData;
                           pTempo++; amTrace(" Update step 25hz: %ld\n",(U32)(*pTempo));
                           pTempo++; amTrace(" Update step 50hz: %ld\n",(U32)(*pTempo));
                           pTempo++; amTrace(" Update step 100hz: %ld\n",(U32)(*pTempo));
                           pTempo++; amTrace(" Update step 200hz: %ld\n",(U32)(*pTempo));
                    }
                }
            ++i;
        }
    }

#ifdef LOAD_TEST
for (U32 i=0;i<pNewSeq->NbOfBlocks;++i){
       amTrace("[LOAD TEST] delta [%lu] type:[%d] size:[%u] bytes\t",pNewSeq->pEvents[i].delta, pNewSeq->pEvents[i].msgType, pNewSeq->pEvents[i].blockSize );
       amTrace("[DATA] ");

       U8 *data = pNewSeq->pEvents[i].pData;

       for(int j=0;j<pNewSeq->pEvents[i].blockSize;++j){
           amTrace("0x%02x ",data[j]);
       }
       amTrace(" [/DATA]\n");
}
#endif
//

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

    if(pSeq->NbOfBlocks==0){

        amMemSet(pSeq,0,sizeof(sNktSeq));
        amFree(pSeq);
        return;

    }else{

        // release linear buffer
        linearBufferFree(&(pSeq->eventBuffer));
        linearBufferFree(&(pSeq->dataBuffer));

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



