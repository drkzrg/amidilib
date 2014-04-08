
#include "config.h"
#include "nkt.h"
#include "timing/mfp.h"
#include "memory.h"
#include "midi_cmd.h"
#include "timing/miditim.h"
#include "midi.h"

// load test
//#define LOAD_TEST 1

// nkt replay
extern void replayNktTC(void);
extern void replayNktTB(void);

static sNktSeq *g_CurrentNktSequence=0;


void getCurrentSequence(sNktSeq **pSeq){
  *pSeq=g_CurrentNktSequence;
}

static BOOL isEOT(volatile const sNktBlock_t *pPtr){
  if(pPtr->msgType==NKT_END) return TRUE;
  return FALSE;
}

static void onEndSequence(){

if(g_CurrentNktSequence){

      if(g_CurrentNktSequence->playMode==NKT_PLAY_ONCE){
          //reset set state to stopped
          //reset song position on all tracks
          g_CurrentNktSequence->playState=NKT_PS_STOPPED;
        }else if(g_CurrentNktSequence->playMode==NKT_PLAY_LOOP){
          g_CurrentNktSequence->playState=NKT_PS_PLAYING;
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
void initSequence(sNktSeq *pSeq){
 g_CurrentNktSequence=0;

if(pSeq!=0){
    U8 mode=0,data=0;
    g_CurrentNktSequence=pSeq;

    pSeq->currentTempo=DEFAULT_MPQN;
    pSeq->currentBPM=DEFAULT_BPM;
    pSeq->timeElapsedInt=0UL;
    pSeq->timeElapsedFrac=0UL;
    pSeq->currentBlockId=0;
    pSeq->timeStep=am_calculateTimeStep(pSeq->currentBPM, pSeq->timeDivision, SEQUENCER_UPDATE_HZ);
    pSeq->playState = getGlobalConfig()->playState;
    pSeq->playMode = getGlobalConfig()->playMode;

#ifdef IKBD_MIDI_SEND_DIRECT
    clearMidiOutputBuffer();
#endif

    getMFPTimerSettings(SEQUENCER_UPDATE_HZ,&mode,&data);

#ifdef DEBUG_BUILD
    amTrace("%dhz update interval, Time step: %d\r\n",SEQUENCER_UPDATE_HZ,pSeq->timeStep);
    amTrace("calculated mode: %d, data: %d\n",mode,data);
#endif

  // installReplayRout(mode, data, replayNktTC);
  installReplayRout(mode, data, replayNktTB);

#ifdef DEBUG_BUILD
 printNktSequenceState();
#endif

  } //endif
 return;
}

void initSequenceManual(sNktSeq *pSeq){
 g_CurrentNktSequence=0;

 if(pSeq!=0){
  U8 mode=0,data=0;
  g_CurrentNktSequence=pSeq;

  pSeq->currentTempo=DEFAULT_MPQN;
  pSeq->currentBPM=DEFAULT_BPM;
  pSeq->timeElapsedInt=0UL;
  pSeq->timeElapsedFrac=0UL;
  pSeq->currentBlockId=0;
  pSeq->timeStep = am_calculateTimeStep(pSeq->currentBPM, pSeq->timeDivision, SEQUENCER_UPDATE_HZ);
  pSeq->playState = getGlobalConfig()->playState;
  pSeq->playMode = getGlobalConfig()->playMode;

  #ifdef IKBD_MIDI_SEND_DIRECT
  clearMidiOutputBuffer();
  #endif

  getMFPTimerSettings(SEQUENCER_UPDATE_HZ,&mode,&data);

  amTrace("%dhz update interval, Time step: %d\r\n",SEQUENCER_UPDATE_HZ,pSeq->timeStep);
  amTrace("calculated mode: %d, data: %d\n",mode,data);

  printNktSequenceState();

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
volatile static U32 timeElapsed=0;
volatile static sNktBlock_t *nktBlk=0;

// single track handler
void updateStepNkt(){
 bStopped=FALSE;

 if(g_CurrentNktSequence==0) return;

 //check sequence state if paused do nothing
  if(g_CurrentNktSequence->playState==NKT_PS_PAUSED) {
    am_allNotesOff(16);

#ifdef IKBD_MIDI_SEND_DIRECT
    flushMidiSendBuffer();
#endif

    return;
  }

  switch(g_CurrentNktSequence->playState){
    case NKT_PS_PLAYING:{
      bStopped=FALSE;
    }break;
    case NKT_PS_STOPPED:{
      //check sequence state if stopped reset position
      //and tempo to default, but only once

      if(bStopped==FALSE){
          bStopped=TRUE;

          g_CurrentNktSequence->currentTempo=DEFAULT_MPQN;
          g_CurrentNktSequence->currentBPM=DEFAULT_BPM;
          g_CurrentNktSequence->timeElapsedInt=0L;
          g_CurrentNktSequence->timeElapsedFrac=0L;

          //reset tempo to initial valueas taken during start(get them from main sequence?)
          g_CurrentNktSequence->timeStep=am_calculateTimeStep(g_CurrentNktSequence->currentBPM,g_CurrentNktSequence->timeDivision, SEQUENCER_UPDATE_HZ);

          //rewind to the first event
          g_CurrentNktSequence->currentBlockId=0;
          nktBlk=&(g_CurrentNktSequence->pEvents[g_CurrentNktSequence->currentBlockId]);

          return;
      }else{
          //do nothing
          return;
      }

    }break;
  };

   bStopped=FALSE; //we replaying, so we have to reset this flag
   nktBlk=(sNktBlock_t *)&(g_CurrentNktSequence->pEvents[g_CurrentNktSequence->currentBlockId]);

   if(nktBlk) bEOTflag=isEOT(nktBlk);

   if(nktBlk!=0&& nktBlk->msgType==NKT_TEMPO_CHANGE){
       //set new tempo
       U32 *tempo=(U32 *)nktBlk->pData;

       g_CurrentNktSequence->currentTempo=*tempo;
       amTrace("[NKT_TEMPO_CHANGE] Set tempo: %lu\n",g_CurrentNktSequence->currentTempo);

       //calculate new timestep
       g_CurrentNktSequence->currentBPM=60000000/g_CurrentNktSequence->currentTempo;
       g_CurrentNktSequence->timeStep=am_calculateTimeStep(g_CurrentNktSequence->currentBPM, g_CurrentNktSequence->timeDivision, SEQUENCER_UPDATE_HZ);

       //next event
       ++(g_CurrentNktSequence->currentBlockId);

       nktBlk=(sNktBlock_t *)&(g_CurrentNktSequence->pEvents[g_CurrentNktSequence->currentBlockId]);
   }

   timeElapsed=g_CurrentNktSequence->timeElapsedInt;
   currentDelta=nktBlk->delta;

   //reset
   bEventSent=FALSE;
   bSend=FALSE;

   if(currentDelta==timeElapsed) bSend=TRUE;

if(bEOTflag==FALSE&&bSend!=FALSE){
    endOfSequence=FALSE;

#ifdef IKBD_MIDI_SEND_DIRECT
        //copy event data to custom buffer
        amTrace("[d: %lu][COPY][%d] ",nktBlk->delta,nktBlk->blockSize);

        U8 *data = nktBlk->pData;
        for(int j=0;j<nktBlk->blockSize;++j){
            amTrace("0x%02x ",data[j]);
        }
        amTrace(" [/COPY] \n");

        amMemCpy(MIDIsendBuffer,nktBlk->pData, nktBlk->blockSize);
        MIDIbytesToSend=nktBlk->blockSize;
        amTrace(" POST MIDIbytesToSend %d\n",MIDIbytesToSend);
#else
        //send to xbios
        amMidiSendData(nktBlk->blockSize,nktBlk->pData);
#endif

   //go to next event
    ++(g_CurrentNktSequence->currentBlockId);
   nktBlk=&(g_CurrentNktSequence->pEvents[g_CurrentNktSequence->currentBlockId]);

   if(nktBlk) bEOTflag=isEOT(nktBlk);

   //check if next events are null and pack buffer until first next non zero delta
   while(bEOTflag!=FALSE&&nktBlk->delta==0){
        //handle event
#ifdef IKBD_MIDI_SEND_DIRECT
       //copy event data to custom buffer
       amTrace("[d: %lu][COPY][%d] ",nktBlk->delta,nktBlk->blockSize);

       U8 *data = nktBlk->pData;
       for(int j=0;j<nktBlk->blockSize;++j){
           amTrace("0x%02x ",data[j]);
       }
       amTrace(" [/COPY] \n");

       amMemCpy(&MIDIsendBuffer[MIDIbytesToSend],nktBlk->pData, nktBlk->blockSize);
       MIDIbytesToSend+=nktBlk->blockSize;
       amTrace(" POST MIDIbytesToSend %d\n",MIDIbytesToSend);
#else
         //send to xbios
        amMidiSendData(nktBlk->blockSize,nktBlk->pData);
#endif

       //go to next event
        ++(g_CurrentNktSequence->currentBlockId);
       nktBlk=(sNktBlock_t *)&(g_CurrentNktSequence->pEvents[g_CurrentNktSequence->currentBlockId]);

       if(nktBlk) bEOTflag=isEOT(nktBlk);
    }

    if(bEOTflag!=FALSE){
      endOfSequence=TRUE;
    }

    bEventSent=TRUE;

  } //endif

    // update
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

} //end updateStepNkt()



sNktSeq *loadSequence(const U8 *pFilePath){

    // create header
    sNktSeq *pNewSeq=amMallocEx(sizeof(sNktSeq),PREFER_TT);

    if(pNewSeq==0){
      printf("Error: Couldn't allocate memory for sequence header.\n");
      return NULL;
    }

    amMemSet(pNewSeq,0,sizeof(sNktSeq));

    pNewSeq->playMode=NKT_PLAY_ONCE;
    pNewSeq->playState=NKT_PS_STOPPED;
    pNewSeq->currentTempo=DEFAULT_MPQN;
    pNewSeq->currentBPM=DEFAULT_BPM;
    pNewSeq->timeDivision=DEFAULT_PPQN;

    //get nb of blocks from file
    FILE *fp=0;

    if(pFilePath){
         // create file header
         printf("Opening NKT file: %s\n",pFilePath);
         amTrace("Opening NKT file: %s\n",pFilePath);

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

   if(pNewSeq->NbOfBlocks==0 || pNewSeq->dataBufferSize==0){
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

    // process blocks, todo allocation on second linear buffer ?
    sNktBlk blk;
    BOOL bFinished=FALSE;
    int i=0;

    U8 *pTempPtr = pNewSeq->pEventDataBuffer;

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

                if(blk.msgType==NKT_TEMPO_CHANGE){
                       U32 *pTempo=(U32 *)pNewSeq->pEvents[i].pData;
                       amTrace("Read tempo: %lu, blocksize: %d\n",(U32)(*pTempo),blk.blockSize);
                }
            }
        ++i;
    }

    fclose(fp);fp=0;

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
       if(g_CurrentNktSequence->playState==NKT_PS_PLAYING)
        return TRUE;
       else
        return FALSE;
 }
return FALSE;
}


void stopSequence(void){
        if(g_CurrentNktSequence!=0){
            if(g_CurrentNktSequence->playState!=NKT_PS_STOPPED){
              g_CurrentNktSequence->playState=NKT_PS_STOPPED;
              printf("Stop sequence\n");
            }
        }

      //all notes off
      am_allNotesOff(16);

    #ifdef IKBD_MIDI_SEND_DIRECT
      flushMidiSendBuffer();
    #endif

    }

void pauseSequence(){
      //printf("Pause/Resume.\n");
      if(g_CurrentNktSequence!=0){
            switch(g_CurrentNktSequence->playState){
                case NKT_PS_PLAYING:{
                    g_CurrentNktSequence->playState=NKT_PS_PAUSED;
                    printf("Pause sequence\n");
                }break;
                case NKT_PS_PAUSED:{
                    g_CurrentNktSequence->playState=NKT_PS_PLAYING;
                }break;
            };
      }
      //all notes off
      am_allNotesOff(16);
    } //pauseSequence

void playSequence(void){

     if(g_CurrentNktSequence!=0){

            if(g_CurrentNktSequence->playState==NKT_PS_STOPPED){
                g_CurrentNktSequence->playState=NKT_PS_PLAYING;
                printf("Play sequence\t");

                switch(g_CurrentNktSequence->playMode){
                    case  NKT_PLAY_ONCE: printf("[ ONCE ]\n"); break;
                    case  NKT_PLAY_LOOP: printf("[ LOOP ]\n"); break;
                    default: printf("\n"); break;

                };
            }
      }
}

void switchReplayMode(void){

 if(g_CurrentNktSequence!=0){
    switch(g_CurrentNktSequence->playMode){
      case NKT_PLAY_ONCE:{
               g_CurrentNktSequence->playMode=NKT_PLAY_LOOP;
               printf("Set replay mode: [ LOOP ]\n");
      }break;
          case NKT_PLAY_LOOP:{
                g_CurrentNktSequence->playMode=NKT_PLAY_ONCE;
                printf("Set replay mode: [ ONCE ]\n");
          }break;
        }
  }
}

// debug stuff
static const U8 *getPlayStateStr(const eNktPlayState state){

    switch(state){
        case NKT_PS_STOPPED:
            return "Stopped";
        break;
        case NKT_PS_PLAYING:
            return "Playing";
        break;
        case NKT_PS_PAUSED:
            return "Paused";
        break;
        default:
            return NULL;
    }
}

static const U8 *getPlayModeStr(const eNktPlayMode mode){
    switch(mode){
        case NKT_PLAY_ONCE:
            return "Play once";
        break;
        case NKT_PLAY_LOOP:
            return "Loop";
        break;
        default:
            return NULL;
    }
}


void printNktSequenceState(){

if(g_CurrentNktSequence){
    printf("Td/PPQN: %u\n",g_CurrentNktSequence->timeDivision);
    printf("Time step: %lu\n",g_CurrentNktSequence->timeStep);
    printf("Time elapsedFrac: %lu\n",g_CurrentNktSequence->timeElapsedFrac);
    printf("\tTime elapsed: %lu\n",g_CurrentNktSequence->timeElapsedInt);
    printf("\tCur BPM: %lu\n",g_CurrentNktSequence->currentBPM);
    printf("\tCur Tempo: %lu\n",g_CurrentNktSequence->currentTempo);
    printf("\tPlay mode: %s\n",getPlayModeStr(g_CurrentNktSequence->playMode));
    printf("\tPlay state: %s\n",getPlayStateStr(g_CurrentNktSequence->playState));
  }

#ifdef DEBUG_BUILD
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

const U8 *getEventTypeName(eNktMsgType type){
    return _arNktEventName[type];
}

