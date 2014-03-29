
#include "config.h"
#include "nkt.h"
#include "timing/mfp.h"
#include "memory.h"
#include "midi_cmd.h"
#include "timing/miditim.h"

// nkt replay
extern void replayNktTC(void);

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
void initSequence(sNktSeq *seq){
 g_CurrentNktSequence=0;

if(seq!=0){
    U8 mode=0,data=0;
    g_CurrentNktSequence=seq;

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

   if(nktBlk!=0&& nktBlk->msgType==NKT_TEMPO_CHANGE){
       //set new tempo
       g_CurrentNktSequence->currentTempo=(U32)*nktBlk->pData;

       //calculate new timestep
       g_CurrentNktSequence->currentBPM=60000000/g_CurrentNktSequence->currentTempo;
       g_CurrentNktSequence->timeStep=am_calculateTimeStep(g_CurrentNktSequence->currentBPM, g_CurrentNktSequence->timeDivision, SEQUENCER_UPDATE_HZ);

       //next event
       ++g_CurrentNktSequence->currentBlockId;
       nktBlk=&(g_CurrentNktSequence->pEvents[g_CurrentNktSequence->currentBlockId]);
   }

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

    amMemCpy(MIDIsendBuffer,nktBlk->pData, nktBlk->blockSize);
    MIDIbytesToSend+=nktBlk->blockSize;
#else
        //send to xbios
        amMidiSendData(nktBlk->blockSize,nktBlk->pData);
#endif

   //go to next event
   ++g_CurrentNktSequence->currentBlockId;
   nktBlk=&(g_CurrentNktSequence->pEvents[g_CurrentNktSequence->currentBlockId]);

   if(nktBlk) bEOTflag=isEOT(nktBlk);

   //check if next events are null and pack buffer until first next non zero delta
   while(bEOTflag!=FALSE&&nktBlk->delta==0){
        //handle event
#ifdef IKBD_MIDI_SEND_DIRECT
       amMemCpy(&MIDIsendBuffer[MIDIbytesToSend],nktBlk->pData, nktBlk->blockSize);
       MIDIbytesToSend+=nktBlk->blockSize;
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
         printf("Opening NKT file to: %s\n",pFilePath);
         fp = fopen(pFilePath, "rb"); //read only

         if(fp==NULL){
             printf("Error: Couldn't open : %s. File doesn't exists.\n",pFilePath);
             amFree((void **)&pNewSeq);
             return NULL;
         }
      }else{
        printf("Error: empty file path\n");
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

   if(pNewSeq->NbOfBlocks==0){
    printf("Error: File %s has no event blocks!\n",pFilePath);
    fclose(fp);fp=0;
    amFree((void **)&pNewSeq);
    return NULL;
   }else{
#ifdef DEBUG_BUILD
        printf("Blocks in sequence: %lu\n",pNewSeq->NbOfBlocks);
#endif
        // allocate contigous/linear memory for pNewSeq->NbOfBlocks events
        if(createLinearBuffer(&(pNewSeq->eventBuffer),pNewSeq->NbOfBlocks*sizeof(sNktBlock_t),PREFER_TT)<0){
            printf("Error: loadSequence() Couldn't allocate memory for temp buffer block buffer.\n");
            fclose(fp); fp=0;
            amFree((void **)&pNewSeq);

            return NULL;
         }
         // alloc memory from linear buffer
         pNewSeq->pEvents=(sNktBlock_t *)linearBufferAlloc(&(pNewSeq->eventBuffer), pNewSeq->NbOfBlocks*sizeof(sNktBlock_t));

         if(pNewSeq->pEvents==0){
             printf("Error: loadSequence() Linear buffer out of memory.\n");
             fclose(fp); fp=0;
             amFree((void **)&pNewSeq);
             return NULL;
         }
    }

    // process blocks, todo allocation on second linear buffer ?
    sNktBlk blk;
    BOOL bFinished=FALSE;
    int i=0;

    while( (!feof(fp))&&(i<pNewSeq->NbOfBlocks)){

            fread(&blk,sizeof(sNktBlk),1,fp);

            pNewSeq->pEvents[i].delta=blk.delta;
            pNewSeq->pEvents[i].msgType=blk.msgType;
            pNewSeq->pEvents[i].blockSize=blk.blockSize;

#ifdef DEBUG_BUILD
            printf("delta [%lu] type:[%d] size:[%u] bytes (0x%x)\n",blk.delta, blk.msgType, blk.blockSize, blk.blockSize );
#endif

            if(pNewSeq->pEvents[i].blockSize!=0){

                // allocate memory for data blk.blockSize
                pNewSeq->pEvents[i].pData=amMallocEx(blk.blockSize,PREFER_TT);

                if( pNewSeq->pEvents[i].pData!=NULL){
                    fread(pNewSeq->pEvents[i].pData,blk.blockSize,1,fp);
                }else{
                    printf("Error: loadSequence() no memory for event block data allocation.\n");
                    fclose(fp);fp=0;
                    amFree((void **)&pNewSeq);
                    return NULL;
                }

            }
        ++i;
    }

    fclose(fp);fp=0;

    return pNewSeq;
}

void destroySequence(sNktSeq *pSeq){

    if(pSeq==0) return;

    if(pSeq->NbOfBlocks==0){
        amMemSet(pSeq,0,sizeof(sNktSeq));
        amFree((void**)&pSeq);
        return;
    }else{

        for(U32 i=0;i<pSeq->NbOfBlocks;++i){
            if(pSeq->pEvents!=0) amFree((void**)&(pSeq->pEvents[i].pData));
        }

        // release linear buffer
        linearBufferFree(&(pSeq->eventBuffer));

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


