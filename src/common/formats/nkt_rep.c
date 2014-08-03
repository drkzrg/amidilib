
#include "nkt.h"
#include "timing/mfp.h"
#include "memory.h"
#include "midi_cmd.h"
#include "timing/miditim.h"
#include "midi.h"
#include "amlog.h"
#include "rol_ptch.h"

#include "minilzo.h" //lzo depack

static sNktSeq *g_CurrentNktSequence=0;

void getCurrentSequence(sNktSeq **pSeq){
  *pSeq=g_CurrentNktSequence;
}

static void onEndSequence(){

if(g_CurrentNktSequence){

  if(g_CurrentNktSequence->sequenceState&NKT_PLAY_ONCE){
    // set state to stopped
    // reset song position on all tracks
    g_CurrentNktSequence->sequenceState&=(~(NKT_PS_PLAYING|NKT_PS_PAUSED));
  }else{
    // loop
    g_CurrentNktSequence->sequenceState&=(~NKT_PS_PAUSED);
    g_CurrentNktSequence->sequenceState|=NKT_PS_PLAYING;
  }

  am_allNotesOff(16);
  g_CurrentNktSequence->timeElapsedInt=0L;
  g_CurrentNktSequence->timeElapsedFrac=0L;

  g_CurrentNktSequence->lastTempo=g_CurrentNktSequence->defaultTempo;
  g_CurrentNktSequence->currentBPM=60000000UL/g_CurrentNktSequence->lastTempo;
  g_CurrentNktSequence->currentBlockId=0;

  // reset all tracks state
  g_CurrentNktSequence->timeStep=am_calculateTimeStep(g_CurrentNktSequence->currentBPM, g_CurrentNktSequence->timeDivision, SEQUENCER_UPDATE_HZ);

#ifdef IKBD_MIDI_SEND_DIRECT
  flushMidiSendBuffer();
#endif
 }

}


// init sequence
void initSequence(sNktSeq *pSeq, U16 initialState){
 g_CurrentNktSequence=0;

if(pSeq!=0){
    U8 mode=0,data=0;
    g_CurrentNktSequence=pSeq;

    pSeq->lastTempo=pSeq->defaultTempo;
    pSeq->currentBPM=60000000UL/g_CurrentNktSequence->lastTempo;
    pSeq->timeElapsedInt=0UL;
    pSeq->timeElapsedFrac=0UL;
    pSeq->currentBlockId=0;
    pSeq->timeStep=am_calculateTimeStep(pSeq->currentBPM, pSeq->timeDivision, SEQUENCER_UPDATE_HZ);
    pSeq->sequenceState = initialState;

#ifdef IKBD_MIDI_SEND_DIRECT
    clearMidiOutputBuffer();
#endif

    getMFPTimerSettings(SEQUENCER_UPDATE_HZ,&mode,&data);

#ifdef DEBUG_BUILD
    amTrace("%dhz update interval, Time step: %d\r\n",SEQUENCER_UPDATE_HZ,pSeq->timeStep);
    amTrace("calculated mode: %d, data: %d\n",mode,data);
#endif

  NktInstallReplayRout(mode, data, MFP_TiC);

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

  pSeq->lastTempo = pSeq->defaultTempo;
  pSeq->currentBPM = DEFAULT_BPM;
  pSeq->timeElapsedInt = 0UL;
  pSeq->timeElapsedFrac = 0UL;
  pSeq->currentBlockId = 0;
  pSeq->timeStep = am_calculateTimeStep(pSeq->currentBPM, pSeq->timeDivision, SEQUENCER_UPDATE_HZ);
  pSeq->sequenceState = state;

  #ifdef IKBD_MIDI_SEND_DIRECT
  clearMidiOutputBuffer();
  #endif

  getMFPTimerSettings(SEQUENCER_UPDATE_HZ,&mode,&data);

  amTrace("%dhz update interval, Time step: %d\r\n",SEQUENCER_UPDATE_HZ,pSeq->timeStep);
  amTrace("calculated mode: %d, data: %d\n",mode,data);

  //printNktSequenceState();

 } //endif
 return;
}
#endif

volatile static BOOL bSend=FALSE;
volatile static BOOL bStopped=FALSE;
volatile static U32 TimeAdd=0;
volatile static U32 tempPPU=0;
volatile static sNktBlock_t *nktBlk=0;

// single track handler
void updateStepNkt(){

 if(g_CurrentNktSequence==0) return;

 //check sequence state if paused do nothing
 if(g_CurrentNktSequence->sequenceState&NKT_PS_PAUSED){
    am_allNotesOff(16);

#ifdef IKBD_MIDI_SEND_DIRECT
    flushMidiSendBuffer();
#endif

    return;
  }

  if(!(g_CurrentNktSequence->sequenceState&NKT_PS_PLAYING)){
    // check sequence state if stopped reset position
    // and tempo to default, but only once

   if(bStopped==FALSE){
      bStopped=TRUE;

      g_CurrentNktSequence->lastTempo=g_CurrentNktSequence->defaultTempo;
      g_CurrentNktSequence->currentBPM=60000000/g_CurrentNktSequence->defaultTempo;
      g_CurrentNktSequence->timeElapsedInt=0L;
      g_CurrentNktSequence->timeElapsedFrac=0L;

      // reset tempo to initial valueas taken during start (get them from main sequence?)
      // calculate new timestep
      tempPPU=g_CurrentNktSequence->currentBPM*g_CurrentNktSequence->timeDivision;

      if(tempPPU<0x10000){
        g_CurrentNktSequence->timeStep=((tempPPU*0x10000)/60)/SEQUENCER_UPDATE_HZ;
      }else{
        g_CurrentNktSequence->timeStep=((tempPPU/60)*0x10000)/SEQUENCER_UPDATE_HZ;
      }

      //rewind to the first event
      g_CurrentNktSequence->currentBlockId=0;
    }
   return;
  }

  bStopped=FALSE;   // we replaying, so we have to reset this flag
  nktBlk=(sNktBlock_t *)&(g_CurrentNktSequence->pEvents[g_CurrentNktSequence->currentBlockId]);

   if(nktBlk!=0 && (nktBlk->msgType&NKT_TEMPO_CHANGE)){
       //set new tempo
       g_CurrentNktSequence->lastTempo=*((U32 *)nktBlk->pData);

       //amTrace("[NKT_TEMPO_CHANGE] Set tempo: %lu\n",g_CurrentNktSequence->lastTempo);

       // calculate new timestep
       g_CurrentNktSequence->currentBPM=60000000UL/g_CurrentNktSequence->lastTempo;
       tempPPU=g_CurrentNktSequence->currentBPM*g_CurrentNktSequence->timeDivision;

       if(tempPPU<0x10000){
           g_CurrentNktSequence->timeStep=((tempPPU*0x10000)/60)/SEQUENCER_UPDATE_HZ;
       }else{
           g_CurrentNktSequence->timeStep=((tempPPU/60)*0x10000)/SEQUENCER_UPDATE_HZ;
       }

       //next event
       ++(g_CurrentNktSequence->currentBlockId);
       return;
   }

   //reset
   bSend=FALSE;

   if(nktBlk->delta==g_CurrentNktSequence->timeElapsedInt) bSend=TRUE;

   if( (!(nktBlk->msgType&NKT_END)) && bSend!=FALSE){

       if(nktBlk->msgType&NKT_TEMPO_CHANGE){
              //set new tempo
              g_CurrentNktSequence->lastTempo=*((U32 *)nktBlk->pData);

              //amTrace("[NKT_TEMPO_CHANGE] Set tempo: %lu\n",g_CurrentNktSequence->lastTempo);

              // calculate new timestep
              g_CurrentNktSequence->currentBPM=60000000UL/g_CurrentNktSequence->lastTempo;
              tempPPU=g_CurrentNktSequence->currentBPM*g_CurrentNktSequence->timeDivision;

              if(tempPPU<0x10000){
                  g_CurrentNktSequence->timeStep=((tempPPU*0x10000)/60)/SEQUENCER_UPDATE_HZ;
              }else{
                  g_CurrentNktSequence->timeStep=((tempPPU/60)*0x10000)/SEQUENCER_UPDATE_HZ;
              }

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
   nktBlk=&(g_CurrentNktSequence->pEvents[g_CurrentNktSequence->currentBlockId]);

   //check if next events are null and pack buffer until first next non zero delta
   while( (!(nktBlk->msgType&NKT_END)) && nktBlk->delta==0){
        //handle event
#ifdef IKBD_MIDI_SEND_DIRECT
       amMemCpy(&MIDIsendBuffer[MIDIbytesToSend],nktBlk->pData, nktBlk->blockSize);
       MIDIbytesToSend+=nktBlk->blockSize;
#else
         //send to xbios
        amMidiSendData(nktBlk->blockSize,nktBlk->pData);
#endif

       //go to next event
        ++(g_CurrentNktSequence->currentBlockId);
       nktBlk=(sNktBlock_t *)&(g_CurrentNktSequence->pEvents[g_CurrentNktSequence->currentBlockId]);
    }

  } //endif

   // update
   g_CurrentNktSequence->timeElapsedFrac += g_CurrentNktSequence->timeStep;
   TimeAdd = g_CurrentNktSequence->timeElapsedFrac >> 16;
   g_CurrentNktSequence->timeElapsedFrac &= 0xffff;

   if(TimeAdd>1) TimeAdd=1;

   // add time elapsed
   if(bSend!=FALSE){
     g_CurrentNktSequence->timeElapsedInt=0;
   }else{
     g_CurrentNktSequence->timeElapsedInt=g_CurrentNktSequence->timeElapsedInt+TimeAdd;
   }

   //check if we have end of sequence
   //on all tracks
   if(nktBlk->msgType&NKT_END){
      onEndSequence();
   }

} //end updateStepNkt()



sNktSeq *loadSequence(const U8 *pFilePath){
    // create header
    sNktSeq *pNewSeq=amMallocEx(sizeof(sNktSeq),PREFER_TT);

    if(pNewSeq==0){
      amTrace("Error: Couldn't allocate memory for sequence header.\n");
      printf("Error: Couldn't allocate memory for sequence header.\n");
      return NULL;
    }

    amMemSet(pNewSeq,0,sizeof(sNktSeq));

    pNewSeq->sequenceState |= NKT_PLAY_ONCE;
    pNewSeq->defaultTempo=DEFAULT_MPQN;
    pNewSeq->lastTempo=DEFAULT_MPQN;
    pNewSeq->currentBPM=DEFAULT_BPM;
    pNewSeq->timeDivision=DEFAULT_PPQN;

    //get nb of blocks from file
    FILE *fp=0;

    if(pFilePath){
         // create file header
         printf("Loading NKT file: %s\n",pFilePath);
         amTrace("Loading NKT file: %s\n",pFilePath);

         fp = fopen(pFilePath, "rb"); //read only

         if(fp==NULL){
             printf("Error: Couldn't open : %s. File doesn't exists.\n",pFilePath);
             amFree((void **)&pNewSeq);
             return NULL;
         }
      }else{
        printf("Error: empty file path\n");
        amTrace("Error: empty file path\n");
        amFree((void **)&pNewSeq);
        return NULL;
      }

    // read header
    sNktHd tempHd;
    amMemSet(&tempHd,0,sizeof(sNktHd));
    fread(&tempHd,sizeof(sNktHd),1,fp);

    if(tempHd.id!=ID_NKT){
         printf("Error: File %s isn't valid!\n",pFilePath);
         fclose(fp); fp=0;

         amFree((void **)&pNewSeq);
         return NULL;
    }


   pNewSeq->NbOfBlocks=tempHd.NbOfBlocks;
   pNewSeq->dataBufferSize=tempHd.NbOfBytesData;
   pNewSeq->timeDivision=tempHd.division;

   if(pNewSeq->NbOfBlocks==0 || pNewSeq->dataBufferSize==0){
        amTrace("Error: File %s has no data or event blocks!\n",pFilePath);
        printf("Error: File %s has no data or event blocks!\n",pFilePath);
        fclose(fp);fp=0;
        amFree((void **)&pNewSeq);
        return NULL;
   }else{
        amTrace("Blocks in sequence: %lu\n",pNewSeq->NbOfBlocks);
        amTrace("Data in bytes: %lu\n",pNewSeq->dataBufferSize);

        // allocate contigous/linear memory for pNewSeq->NbOfBlocks events
        if(createLinearBuffer(&(pNewSeq->eventBuffer),pNewSeq->NbOfBlocks*sizeof(sNktBlock_t),PREFER_TT)<0){
            printf("Error: loadSequence() Couldn't allocate memory for event block buffer.\n");
            amTrace("Error: loadSequence() Couldn't allocate memory for event block buffer.\n");

            fclose(fp); fp=0;
            amFree((void **)&pNewSeq);
            return NULL;
         }

         // alloc memory for event blocks from linear buffer
         pNewSeq->pEvents=(sNktBlock_t *)linearBufferAlloc(&(pNewSeq->eventBuffer), pNewSeq->NbOfBlocks*sizeof(sNktBlock_t));

         if(pNewSeq->pEvents==0){
             printf("Error: loadSequence() Linear buffer out of memory.\n");
             amTrace("Error: loadSequence() Linear buffer out of memory.\n");

             fclose(fp); fp=0;
             amFree((void **)&pNewSeq);
             return NULL;
         }

         amTrace("Allocated %lu kb for event block buffer\n",(pNewSeq->NbOfBlocks*sizeof(sNktBlock_t))/1024);

         if(createLinearBuffer(&(pNewSeq->dataBuffer),pNewSeq->dataBufferSize,PREFER_TT)<0){
             printf("Error: loadSequence() Couldn't allocate memory for temp data buffer. \n");
             amTrace("Error: loadSequence() Couldn't allocate memory for temp data buffer. \n");

             fclose(fp); fp=0;

             // destroy block buffer
             destroyLinearBuffer(&(pNewSeq->eventBuffer));
             amFree((void **)&pNewSeq);

             return NULL;
         }

       // alloc memory for data buffer from linear allocator
       pNewSeq->pEventDataBuffer = linearBufferAlloc(&(pNewSeq->dataBuffer), pNewSeq->dataBufferSize);

       amTrace("Allocated %lu kb for event data buffer\n",(pNewSeq->dataBufferSize)/1024);

       if(pNewSeq->pEventDataBuffer==0){
           printf("Error: loadSequence() Linear buffer out of memory.\n");
           amTrace("Error: loadSequence() Linear buffer out of memory.\n");

           // destroy block buffer
           destroyLinearBuffer(&(pNewSeq->eventBuffer));
           destroyLinearBuffer(&(pNewSeq->dataBuffer));

           fclose(fp); fp=0;
           amFree((void **)&pNewSeq);
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
        rewind(fp);
        fseek(fp,sizeof(sNktHd),SEEK_SET);

        // allocate temp buffer for unpacked data
        U32 destSize=tempHd.NbOfBlocks * sizeof(sNktBlock_t) + tempHd.NbOfBytesData;

        fprintf(stderr,"[LZO] packed: %lu, unpacked: %lu\n", tempHd.bytesPacked, destSize);

        void *pDepackBuf = amMallocEx(tempHd.bytesPacked,PREFER_TT); //packed data buffer
        void *pOutputBuf = amMallocEx(destSize,PREFER_TT);    // depacked midi data
        void *pWrkBuffer = amMallocEx(destSize/16,PREFER_TT); // lzo work buffer

        if(pDepackBuf!=0&&pOutputBuf!=0&&pWrkBuffer!=0){
         amMemSet(pDepackBuf,0,tempHd.bytesPacked);
         amMemSet(pOutputBuf,0,destSize);
         amMemSet(pWrkBuffer,0,destSize/16);

         if(fread(pDepackBuf,1,tempHd.bytesPacked,fp)==tempHd.bytesPacked){

             // decompress data
             fprintf(stderr,"[LZO] Decompressing ...\n");

                 if(lzo1x_decompress_safe(pDepackBuf,tempHd.bytesPacked,pOutputBuf,&destSize,pWrkBuffer)==LZO_E_OK){
                     // process data
                     fprintf(stderr,"[LZO] Block decompressed: %lu, packed: %lu\n",destSize,tempHd.bytesPacked);
                     U8 *pData = (U8 *)pOutputBuf;

                     // process decompressed data directly from file
                     while(pData!=(pData+destSize)&&i<pNewSeq->NbOfBlocks){
                             U32 delta=0;
                             U8 count=0;

                             delta=readVLQ(pData,&count);
                             amTrace("READ delta byte count:[%d] decoded delta:[%lu] \n", count,delta );
                             pData+=count;

                             // read msg block
                             //  fread(&blk,sizeof(sNktBlk),1,fp);
                             sNktBlk *pTemp=(sNktBlk *)pData;
                             pNewSeq->pEvents[i].delta=delta;
                             pNewSeq->pEvents[i].msgType=pTemp->msgType;
                             pNewSeq->pEvents[i].blockSize=pTemp->blockSize;

                             pData+=sizeof(sNktBlk);
                             amTrace("READ delta [%lu] type:[%d] size:[%u] bytes\n", delta, pTemp->msgType, pTemp->blockSize );

                             if(pNewSeq->pEvents[i].blockSize!=0){

                                 // assign buffer memory pointer for data, size blk.blockSize
                                 pNewSeq->pEvents[i].pData=pTempPtr;
                                 pTempPtr+=pTemp->blockSize;

                                 amMemCpy(pNewSeq->pEvents[i].pData,pData,pTemp->blockSize);
                                 pData+=pTemp->blockSize;
#ifdef DEBUG_BUILD
                                 if(blk.msgType&NKT_TEMPO_CHANGE){
                                        U32 *pTempo=(U32 *)pNewSeq->pEvents[i].pData;
                                        amTrace("Read tempo: %lu, blocksize: %d\n",(U32)(*pTempo),blk.blockSize);
                                 }
#endif
                             }
                         ++i;
                     }



                     fprintf(stderr,"[LZO] OK\n");

                 }else{
                     fprintf(stderr,"[LZO] Data decompression error.\n");
                 }
            }else{
             fprintf(stderr,"[LZO] Data read failed ..\n");
          }

        // deallocate temp buffers
            amFree((void**)&pWrkBuffer);
            amFree((void**)&pOutputBuf);
            amFree((void**)&pDepackBuf);

        }else{
            fprintf(stderr,"[LZO] Error: Couldn't allocate work buffers.\n");
            getchar();
			fclose(fp);fp=0;
            return NULL;
        }
    }else{
        // process decompressed data directly from file
        while( (!feof(fp))&&(i<pNewSeq->NbOfBlocks)){
                U32 tempDelta,delta=0;
                U8 count=0;

                fread(&tempDelta,sizeof(U32),1,fp);
                delta=readVLQ((U8*)&tempDelta,&count);
                amTrace("READ delta byte count:[%d] decoded delta:[%lu] \n", count,delta );

                // rewind depending how many bytes were read from VLQ (size of former read - count of bytes read)
                fseek(fp,-(sizeof(U32)-count),1);

                // read msg block
                fread(&blk,sizeof(sNktBlk),1,fp);
                pNewSeq->pEvents[i].delta=delta;
                pNewSeq->pEvents[i].msgType=blk.msgType;
                pNewSeq->pEvents[i].blockSize=blk.blockSize;

                amTrace("READ delta [%lu] type:[%d] size:[%u] bytes\n", delta, blk.msgType, blk.blockSize );

                if(pNewSeq->pEvents[i].blockSize!=0){

                    // assign buffer memory pointer for data, size blk.blockSize
                    pNewSeq->pEvents[i].pData=pTempPtr;
                    pTempPtr+=blk.blockSize;

                    fread(pNewSeq->pEvents[i].pData,blk.blockSize,1,fp);

                    /*if(blk.msgType==NKT_TEMPO_CHANGE){
                           U32 *pTempo=(U32 *)pNewSeq->pEvents[i].pData;
                           amTrace("Read tempo: %lu, blocksize: %d\n",(U32)(*pTempo),blk.blockSize);
                    }*/
                }
            ++i;
        }

        fclose(fp);fp=0;
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

 return pNewSeq;
}

void destroySequence(sNktSeq *pSeq){

    if(pSeq==0) return;

    if(pSeq->NbOfBlocks==0){
        amMemSet(pSeq,0,sizeof(sNktSeq));
        amFree((void**)&pSeq);
        return;
    }else{

        // release linear buffer
        linearBufferFree(&(pSeq->eventBuffer));
        linearBufferFree(&(pSeq->dataBuffer));

        //clear struct
        amMemSet(pSeq,0,sizeof(sNktSeq));
        amFree((void**)&pSeq);
        return;
    }
}


////////////////////////////////////////////////// replay control

BOOL isSequencePlaying(void){

 if(g_CurrentNktSequence!=0){
     U16 state=g_CurrentNktSequence->sequenceState;
     if(state&NKT_PS_PLAYING&&!(state&NKT_PS_PAUSED))
        return TRUE;
       else
        return FALSE;
 }

 return FALSE;
}


void stopSequence(void){
 if(g_CurrentNktSequence!=0){

    if((g_CurrentNktSequence->sequenceState&NKT_PS_PLAYING)||(g_CurrentNktSequence->sequenceState&NKT_PS_PAUSED)){
       g_CurrentNktSequence->sequenceState&=(~(NKT_PS_PLAYING|NKT_PS_PAUSED));
       printf("Stop sequence\n");
    }

    //all notes off
    am_allNotesOff(16);

    #ifdef IKBD_MIDI_SEND_DIRECT
      flushMidiSendBuffer();
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

           printf("Pause sequence\n");
           return;
          }else if(!(g_CurrentNktSequence->sequenceState&NKT_PS_PLAYING)&&(state&NKT_PS_PAUSED) ){
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

         printf("Play sequence\t");

         if(g_CurrentNktSequence->sequenceState&NKT_PLAY_ONCE){
           printf("[ ONCE ]\n");
         }else{
           printf("[ LOOP ]\n");
         }
      }
 }
}

void switchReplayMode(void){

 if(g_CurrentNktSequence!=0){
     if(g_CurrentNktSequence->sequenceState&NKT_PLAY_ONCE){
        g_CurrentNktSequence->sequenceState&=(~NKT_PLAY_ONCE);
        printf("Set replay mode: [ LOOP ]\n");
     }else{
        g_CurrentNktSequence->sequenceState|=NKT_PLAY_ONCE;
        printf("Set replay mode: [ ONCE ]\n");
     }
  }
}

void NktInit(const eMidiDeviceType devType, const U8 channel){

    initDebug("nktlog.log");

    // now depending on the connected device type and chosen operation mode
    // set appropriate channel
    // prepare device for receiving messages

     setupMidiDevice(devType,channel);
}


void NktDeinit(){
#ifdef IKBD_MIDI_SEND_DIRECT
    // send content of midi buffer to device
    flushMidiSendBuffer();
#endif

    deinitDebug();
}

#ifdef DEBUG_BUILD

// debug stuff
static const U8 *getSequenceStateStr(const U16 state){

 if( !(state&NKT_PS_PLAYING) && (state&NKT_PS_PAUSED) ){
    return "Paused";
 }else if(state&NKT_PS_PLAYING && !state&NKT_PS_PAUSED){
    return "Playing";
 }else if(!(state&NKT_PS_PLAYING)){
    return "Stopped...";
 }
}


void printNktSequenceState(){

if(g_CurrentNktSequence){
    printf("Td/PPQN: %u\n",g_CurrentNktSequence->timeDivision);
    printf("Time step: %lu\n",g_CurrentNktSequence->timeStep);
    printf("Time elapsedFrac: %lu\n",g_CurrentNktSequence->timeElapsedFrac);
    printf("\tTime elapsed: %lu\n",g_CurrentNktSequence->timeElapsedInt);
    printf("\tCur BPM: %lu\n",g_CurrentNktSequence->currentBPM);
    printf("\tDefault Tempo: %lu\n",g_CurrentNktSequence->defaultTempo);
    printf("\tLast Tempo: %lu\n",g_CurrentNktSequence->lastTempo);
    printf("\tSequence state: 0x%x\n",getSequenceStateStr(g_CurrentNktSequence->sequenceState));
  }

 printMidiSendBufferState();
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


