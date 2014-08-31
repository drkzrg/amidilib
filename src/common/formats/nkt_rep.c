
#include "nkt.h"
#include "memory.h"
#include "amlog.h"

#include "timing/mfp.h"
#include "timing/miditim.h"

#include "midi.h"
#include "midi_cmd.h"
#include "rol_ptch.h"

#include "minilzo.h" //lzo depack, TODO: add compilation flag to remove lzo during compilation time

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

  g_CurrentNktSequence->currentTempo.tempo=g_CurrentNktSequence->defaultTempo.tempo;
  g_CurrentNktSequence->currentBlockId=0;

  // reset all tracks state
  g_CurrentNktSequence->timeStep=g_CurrentNktSequence->defaultTempo.tuTable[g_CurrentNktSequence->currentUpdateFreq];

#ifdef IKBD_MIDI_SEND_DIRECT
  flushMidiSendBuffer();
#endif
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
     // precalculate valuies for different update steps

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

#ifdef IKBD_MIDI_SEND_DIRECT
    clearMidiOutputBuffer();
#endif

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

volatile static BOOL bStopped=FALSE;
volatile static U32 TimeAdd=0;
volatile static sNktBlock_t *nktBlk=0;

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

  if(g_CurrentNktSequence->sequenceState&NKT_PS_PLAYING){

      bStopped=FALSE;   // we replaying, so we have to reset this flag

      // get sequence block
      nktBlk=(sNktBlock_t *)&(g_CurrentNktSequence->pEvents[g_CurrentNktSequence->currentBlockId]);

      // track end?
      if(nktBlk->msgType&NKT_END){
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
      printf("Error: Couldn't allocate memory for sequence header.\n");
      return NULL;
    }

    amMemSet(pNewSeq,0,sizeof(sNktSeq));
	
    pNewSeq->currentUpdateFreq=NKT_U200HZ;
    pNewSeq->sequenceState |= NKT_PLAY_ONCE;
    pNewSeq->defaultTempo.tempo=DEFAULT_MPQN;
    pNewSeq->currentTempo.tempo=DEFAULT_MPQN;
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
             amFree(pNewSeq);
             return NULL;
         }
      }else{
        printf("Error: empty file path\n");
        amTrace("Error: empty file path\n");
        amFree(pNewSeq);
        return NULL;
      }

    // read header
    sNktHd tempHd;
    amMemSet(&tempHd,0,sizeof(sNktHd));
    fread(&tempHd,sizeof(sNktHd),1,fp);

    if(tempHd.id!=ID_NKT){
         printf("Error: File %s isn't valid!\n",pFilePath);
         fclose(fp); fp=0;

         amFree(pNewSeq);
         return NULL;
    }

   pNewSeq->NbOfBlocks=tempHd.NbOfBlocks;
   pNewSeq->dataBufferSize=tempHd.NbOfBytesData;
   pNewSeq->timeDivision=tempHd.division;

   if(pNewSeq->NbOfBlocks==0 || pNewSeq->dataBufferSize==0){
        amTrace("Error: File %s has no data or event blocks!\n",pFilePath);
        printf("Error: File %s has no data or event blocks!\n",pFilePath);
        fclose(fp);fp=0;
        amFree(pNewSeq);
        return NULL;
   }else{
        amTrace("Blocks in sequence: %lu\n",pNewSeq->NbOfBlocks);
        amTrace("Data in bytes: %lu\n",pNewSeq->dataBufferSize);

        // allocate contigous/linear memory for pNewSeq->NbOfBlocks events
        if(createLinearBuffer(&(pNewSeq->eventBuffer),pNewSeq->NbOfBlocks*sizeof(sNktBlock_t),PREFER_TT)<0){
            printf("Error: loadSequence() Couldn't allocate memory for event block buffer.\n");
            amTrace("Error: loadSequence() Couldn't allocate memory for event block buffer.\n");

            fclose(fp); fp=0;
            amFree(pNewSeq);
            return NULL;
         }

         // alloc memory for event blocks from linear buffer
         pNewSeq->pEvents=(sNktBlock_t *)linearBufferAlloc(&(pNewSeq->eventBuffer), pNewSeq->NbOfBlocks*sizeof(sNktBlock_t));

         if(pNewSeq->pEvents==0){
             printf("Error: loadSequence() Linear buffer out of memory.\n");
             amTrace("Error: loadSequence() Linear buffer out of memory.\n");

             fclose(fp); fp=0;
             amFree(pNewSeq);
             return NULL;
         }

         amTrace("Allocated %lu kb for event block buffer\n",(pNewSeq->NbOfBlocks*sizeof(sNktBlock_t))/1024);

         if(createLinearBuffer(&(pNewSeq->dataBuffer),pNewSeq->dataBufferSize,PREFER_TT)<0){
             printf("Error: loadSequence() Couldn't allocate memory for temp data buffer. \n");
             amTrace("Error: loadSequence() Couldn't allocate memory for temp data buffer. \n");

             fclose(fp); fp=0;

             // destroy block buffer
             destroyLinearBuffer(&(pNewSeq->eventBuffer));
             amFree(pNewSeq);

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

                     fprintf(stderr,"[LZO] OK\n");

                 }else{
                     fprintf(stderr,"[LZO] Data decompression error.\n");
                 }
            }else{
             fprintf(stderr,"[LZO] Data read failed ..\n");
          }

        // deallocate temp buffers
            amFree(pWrkBuffer);
            amFree(pOutputBuf);
            amFree(pDepackBuf);

        }else{
			amFree(pWrkBuffer);
            amFree(pOutputBuf);
            amFree(pDepackBuf);
		
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
                amTrace("[R] delta:[%lu], byte count:[%d]\t", count,delta );

                // rewind depending how many bytes were read from VLQ (size of former read - count of bytes read)
                fseek(fp,-(sizeof(U32)-count),1);

                // read msg block
                fread(&blk,sizeof(sNktBlk),1,fp);
                pNewSeq->pEvents[i].delta=delta;
                pNewSeq->pEvents[i].msgType=blk.msgType;
                pNewSeq->pEvents[i].blockSize=blk.blockSize;

                amTrace("[RB] delta [%lu] type:[%d] size:[%u] bytes\n", delta, blk.msgType, blk.blockSize );

                if(pNewSeq->pEvents[i].blockSize!=0){

                    // assign buffer memory pointer for data, size blk.blockSize
                    pNewSeq->pEvents[i].pData=pTempPtr;
                    pTempPtr+=blk.blockSize;

                    fread(pNewSeq->pEvents[i].pData,blk.blockSize,1,fp);

                    if(blk.msgType==NKT_TEMPO_CHANGE){
                           U32 *pTempo=(U32 *)pNewSeq->pEvents[i].pData;
                           pTempo++;amTrace(" Update step 25hz: %ld\n",(U32)(*pTempo));
                           pTempo++;amTrace(" Update step 50hz: %ld\n",(U32)(*pTempo));
                           pTempo++;amTrace(" Update step 100hz: %ld\n",(U32)(*pTempo));
                           pTempo++;amTrace(" Update step 200hz: %ld\n",(U32)(*pTempo));
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
 fclose(fp);fp=0;
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
 }else if(state&NKT_PS_PLAYING && (!(state&NKT_PS_PAUSED))){
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
    printf("\tDefault Tempo: %lu\n",g_CurrentNktSequence->defaultTempo.tempo);
    printf("\tLast Tempo: %lu\n",g_CurrentNktSequence->currentTempo.tempo);
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


