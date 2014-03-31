
// converts sequence to custom nok, binary format

#include "nkt.h"
#include "seq2nkt.h"

#include "midi_cmd.h"
#include "timing/miditim.h"
#include "memory/linalloc.h"


void Nkt_CreateHeader(sNktHd* header, const sSequence_t *pSeqData, const BOOL bCompress){
    WriteInt(&header->id,ID_NKT);
    WriteInt(&header->NbOfBlocks, 0);
    WriteShort(&header->division, pSeqData->timeDivision);
    WriteShort(&header->bPacked, bCompress);
    WriteShort(&header->version, 1);
}

static void processSeqEvent(sEventList *pCurEvent, U8 tab[],U32 *bufPos, U32 *bufDataSize){
    // write data to out data block

    switch(pCurEvent->eventBlock.type){
        case T_NOTEON:{
            // dump data
            amTrace("T_NOTEON \n");
            sNoteOn_EventBlock_t *pEventBlk=(sNoteOn_EventBlock_t *)&(pCurEvent->eventBlock);

            // write to output buffer
            tab[(*bufPos)++]=(EV_NOTE_ON<<4)|pEventBlk->ubChannelNb;
            tab[(*bufPos)++]=pEventBlk->eventData.noteNb;
            tab[(*bufPos)++]=pEventBlk->eventData.velocity;
            (*bufDataSize)+=3;

            return;
        } break;
        case T_NOTEOFF:{
            // dump data
            amTrace("T_NOTEOFF \n");
            sNoteOff_EventBlock_t *pEventBlk=(sNoteOff_EventBlock_t *)&(pCurEvent->eventBlock);           
            tab[(*bufPos)++]=(EV_NOTE_OFF<<4)|(pEventBlk->ubChannelNb);
            tab[(*bufPos)++]=pEventBlk->eventData.noteNb;
            tab[(*bufPos)++]=pEventBlk->eventData.velocity;
            (*bufDataSize)+=3;
            return;
        } break;
        case T_NOTEAFT:{
            // dump data
            amTrace("T_NOTEAFT \n");
            sNoteAft_EventBlock_t *pEventBlk=(sNoteAft_EventBlock_t *)&(pCurEvent->eventBlock);
            tab[(*bufPos)++]=(EV_NOTE_AFTERTOUCH<<4)|pEventBlk->ubChannelNb;
            tab[(*bufPos)++]=pEventBlk->eventData.noteNb;
            tab[(*bufPos)++]=pEventBlk->eventData.pressure;
            (*bufDataSize)+=3;

            return;
        } break;
        case T_CONTROL:{
            amTrace("T_CONTROL \n");
            // program change (dynamic according to connected device)
            sController_EventBlock_t *pEventBlk=(sController_EventBlock_t *)&(pCurEvent->eventBlock);

            tab[(*bufPos)++]=(EV_CONTROLLER<<4)|pEventBlk->ubChannelNb;
            tab[(*bufPos)++]=pEventBlk->eventData.controllerNb;
            tab[(*bufPos)++]=pEventBlk->eventData.value;
            tab[(*bufPos)++]=0x00;
            (*bufDataSize)+=4;
            return;
        } break;
        case T_PRG_CH:{
        // program change (dynamic according to connected device)
         amTrace("T_PRG_CH \n");
            sPrgChng_EventBlock_t *pEventBlk=(sPrgChng_EventBlock_t *)&(pCurEvent->eventBlock);
            tab[(*bufPos)++]=(EV_PROGRAM_CHANGE<<4)|pEventBlk->ubChannelNb;
            tab[(*bufPos)++]=pEventBlk->eventData.programNb;
            (*bufDataSize)+=2;
            return;
        } break;
        case T_CHAN_AFT:{
            // dump data
         amTrace("T_CHAN_AFT \n");
            sChannelAft_EventBlock_t *pEventBlk=(sChannelAft_EventBlock_t *)&(pCurEvent->eventBlock);
            tab[(*bufPos)++] = (EV_CHANNEL_AFTERTOUCH<<4) | pEventBlk->ubChannelNb;
            tab[(*bufPos)++] = pEventBlk->eventData.pressure;
            (*bufDataSize)+=2;
            return;
        } break;
        case T_PITCH_BEND:{
            // dump data
         amTrace("T_PITCH_BEND \n");
            sPitchBend_EventBlock_t  *pEventBlk=(sPitchBend_EventBlock_t *)&(pCurEvent->eventBlock);
            copy_pitch_bend_2(pEventBlk->ubChannelNb,pEventBlk->eventData.LSB,pEventBlk->eventData.MSB);

            tab[(*bufPos)++]=(EV_PITCH_BEND<<4)|pEventBlk->ubChannelNb;
            tab[(*bufPos)++]=pEventBlk->eventData.LSB; //LSB
            tab[(*bufPos)++]=pEventBlk->eventData.MSB; //MSB
            (*bufDataSize)+=3;
            return;
        } break;
        case T_META_SET_TEMPO:{
            //set tempo
        amTrace("T_META_SET_TEMPO \n");
           // sTempo_EventBlock_t  *pEventBlk=(sTempo_EventBlock_t  *)&(pCurEvent->eventBlock);
            // skip
            return;
        } break;
        case T_META_EOT:{
             // skip
           amTrace("T_META_EOT \n");
           // sEot_EventBlock_t *pEventBlk=(sEot_EventBlock_t *)&(pCurEvent->eventBlock);
            return;
        } break;
        case T_META_CUEPOINT:{
            //skip
          amTrace("T_META_CUEPOINT \n");
            //sCuePoint_EventBlock_t *pEventBlk=(sCuePoint_EventBlock_t *)&(pCurEvent->eventBlock);
            return;
        } break;
        case T_META_MARKER:{
            //skip
         amTrace("T_META_MARKER \n");
            //sMarker_EventBlock_t *pEventBlk=(sMarker_EventBlock_t *)&(pCurEvent->eventBlock);
            return;
        } break;
        case T_META_SET_SIGNATURE:{
        //skip
         amTrace("T_META_SET_SIGNATURE \n");
            //sTimeSignature_EventBlock_t *pEventBlk=(sTimeSignature_EventBlock_t *)&(pCurEvent->eventBlock);
             return;
        } break;
        case T_SYSEX:{
        // copy data
         amTrace("T_SYSEX \n");
        // format depends on connected device

         sSysEX_EventBlock_t *pEventBlk=(sSysEX_EventBlock_t *)&(pCurEvent->eventBlock);
          amTrace((const U8*)"Copy SysEX Message.\n");
          //TODO: check buffer overflow
          amMemCpy(&tab[(*bufPos)],pEventBlk->pBuffer,pEventBlk->bufferSize);
          (*bufDataSize)+=pEventBlk->bufferSize;
        } break;
    default:
        // error not handled
        amTrace("Error: processSeqEvent() error not handled\n");
        printf("Error: processSeqEvent() error not handled\n");
        return;
        break;
  };
};


U8 seq2nktMap[]={
    NKT_MIDIDATA,       //  T_NOTEON=0,
    NKT_MIDIDATA,       //	T_NOTEOFF,
    NKT_MIDIDATA,       //	T_NOTEAFT,
    NKT_MIDIDATA,       //	T_CONTROL,
    NKT_MIDIDATA,       //	T_PRG_CH,
    NKT_MIDIDATA,       //	T_CHAN_AFT,
    NKT_MIDIDATA,       //	T_PITCH_BEND,
    NKT_TEMPO_CHANGE,   //	T_META_SET_TEMPO,
    NKT_END,            //	T_META_EOT,
    NKT_TRIGGER,        //	T_META_CUEPOINT,
    NKT_TRIGGER,        //	T_META_MARKER,
    NKT_MIDIDATA,       //	T_META_SET_SIGNATURE,
    NKT_MIDIDATA       //	T_SYSEX,
};




static S32 handleSingleTrack(const sSequence_t *pSeq, const BOOL bCompress, FILE **file,U32 *blocksWritten, U32 *bytesWritten){

    printf("Processing single track ...\n");
    U8 tempBuffer[32 * 1024]={0};
    U32 bufPos=0;
    U32 bufDataSize=0;

    sTrack_t *pTrack=pSeq->arTracks[0];
    sEventList *eventPtr=pTrack->pTrkEventList;
    U32 currDelta=0;
    sNktBlk stBlock;



    while(eventPtr!=NULL){

        // dump data
        if(eventPtr->eventBlock.uiDeltaTime==currDelta){
           // if event is tempo related then create event with curent delta
           // and go to next event
            bufPos=0;
            bufDataSize=0;

            stBlock.delta=currDelta;

            //skip uninteresting events
            while((eventPtr!=NULL) &&( (eventPtr->eventBlock.type==T_META_CUEPOINT) || (eventPtr->eventBlock.type==T_META_MARKER) )){
                printf("Skip event >> %d\n",eventPtr->eventBlock.type);
                eventPtr=eventPtr->pNext;
            }

            if((eventPtr!=NULL) &&(eventPtr->eventBlock.type==T_META_SET_TEMPO)){
                U32 tempo = ((sTempo_EventBlock_t *)(eventPtr->eventBlock.dataPtr))->eventData.tempoVal;

                amTrace("Write Set Tempo: %lu event\n",tempo);

                stBlock.delta=currDelta;
                stBlock.blockSize=sizeof(U32);
                stBlock.msgType=(U16)seq2nktMap[eventPtr->eventBlock.type];

                //write block to file
                if(file!=NULL){
                   *bytesWritten+=fwrite(&stBlock,sizeof(stBlock),1,*file);
                   *bytesWritten+=fwrite(&tempo,sizeof(U32),1,*file);
                 }
                ++(*blocksWritten);

                amTrace("delta [%lu] type:[%d] size:[%u] bytes \n",stBlock.delta, stBlock.msgType, stBlock.blockSize );
                eventPtr=eventPtr->pNext;
            }

            if((eventPtr!=NULL) &&(eventPtr->eventBlock.type==T_META_EOT)){

                amTrace("Write End of Track \n");

                stBlock.delta=currDelta;
                stBlock.blockSize=0;
                stBlock.msgType=(U16)seq2nktMap[eventPtr->eventBlock.type];

                //write block to file
                if(file!=NULL){
                    *bytesWritten+=fwrite(&stBlock,sizeof(stBlock),1,*file);
                    //no data to write
                }
                ++(*blocksWritten);
                eventPtr=eventPtr->pNext;

            }

            if(eventPtr!=0){

                // process events as normal
                processSeqEvent(eventPtr, tempBuffer, &bufPos, &bufDataSize);

                stBlock.msgType=(U16)seq2nktMap[eventPtr->eventBlock.type];

                // next event
                eventPtr=eventPtr->pNext;

                //check next events, dump until delta != 0
                while(eventPtr!=NULL&&eventPtr->eventBlock.uiDeltaTime==0){


                    //skip uninteresting events
                    while((eventPtr!=NULL) &&( (eventPtr->eventBlock.type==T_META_CUEPOINT) || (eventPtr->eventBlock.type==T_META_MARKER) )){
                        printf("Skip event >> %d delta 0\n",eventPtr->eventBlock.type);
                        eventPtr=eventPtr->pNext;
                    }

                    if((eventPtr!=NULL) &&(eventPtr->eventBlock.type==T_META_SET_TEMPO)){
                        printf("Write Set Tempo: %lu, event delta 0\n",((sTempo_EventBlock_t *)(eventPtr->eventBlock.dataPtr))->eventData.tempoVal);
                        U32 tempo = ((sTempo_EventBlock_t *)(eventPtr->eventBlock.dataPtr))->eventData.tempoVal;

                        amTrace("Write Set Tempo: %lu event\n",tempo);

                        stBlock.delta=currDelta;
                        stBlock.blockSize=sizeof(U32);
                        stBlock.msgType=(U16)seq2nktMap[eventPtr->eventBlock.type];

                        //write block to file
                        if(file!=NULL){
                           *bytesWritten+=fwrite(&stBlock,sizeof(stBlock),1,*file);
                           *bytesWritten+=fwrite(&tempo,sizeof(U32),1,*file);
                         }
                        ++(*blocksWritten);

                        amTrace("delta [%lu] type:[%d] size:[%u] bytes \n",stBlock.delta, stBlock.msgType, stBlock.blockSize );
                        eventPtr=eventPtr->pNext;
                    }

                    if((eventPtr!=NULL) &&(eventPtr->eventBlock.type==T_META_EOT)){
                        printf("Write End of Track, event delta 0\n");

                        stBlock.delta=currDelta;
                        stBlock.blockSize=0;
                        stBlock.msgType=(U16)seq2nktMap[eventPtr->eventBlock.type];

                        //write block to file
                        if(file!=NULL){
                            *bytesWritten+=fwrite(&stBlock,sizeof(stBlock),1,*file);
                            //no data to write
                        }
                        ++(*blocksWritten);

                        eventPtr=eventPtr->pNext;
                    }

                    if(eventPtr!=NULL) {
                        processSeqEvent(eventPtr, tempBuffer, &bufPos, &bufDataSize);

                        // next event
                        eventPtr=eventPtr->pNext;
                    }

                }; //end delta == 0 while


                // dump midi event block to memory
                if(bufPos>0){
                    stBlock.blockSize=bufDataSize;

                    amTrace("[DATA] ");
                        U8 *data = &tempBuffer[0];
                        for(int j=0;j<bufDataSize;++j){
                            amTrace("0x%02x ",data[j]);
                        }
                    amTrace(" [/DATA]\n");

                    //write block to file
                    if(file!=NULL){
                            amTrace("Write block size %d\n",stBlock.blockSize);
                            *bytesWritten+=fwrite(&stBlock, sizeof(sNktBlk), 1, *file);
                            *bytesWritten+=fwrite((const void *)tempBuffer,stBlock.blockSize,1,*file);
                    }
                    ++(*blocksWritten);

                    amTrace("delta [%lu] type:[%d] size:[%u] bytes \n",stBlock.delta, stBlock.msgType, stBlock.blockSize);

                    //clear buffer
                    bufDataSize=0;
                    bufPos=0;
                    amMemSet(tempBuffer,0,32 * 1024);

                }else{
                    printf(" Nothing to dump... \n");
                }


                if(eventPtr) {
                    currDelta=eventPtr->eventBlock.uiDeltaTime; //get next delta
                    amTrace("Next delta: %lu\n",currDelta);
                }

            }//end null check




        }; // end while



    }; //end while end of sequence

    printf("Event blocks written: %lu, total bytes of data written %lu\n",*blocksWritten,*bytesWritten);
 return 0;
}

static S32 handleMultiTrack(const sSequence_t *pSeq, const BOOL bCompress, FILE **file, U32 *bytesWritten, U32 *blocksWritten){
//TODO
    printf("Processing multi track sequence\n");



    printf("Event blocks written: %lu\n",*blocksWritten);
 return 0;
}


//converts sequence to nkt file, optionally writes file named out
S32 Seq2NktFile(const sSequence_t *pSeq, const U8 *pOutFileName, const BOOL bCompress){
U32 bytes_written = 0;
U32 blocks_written = 0;
BOOL error=FALSE;
sNktHd nktHead;

FILE* file=0;

 if(pOutFileName){
      // create file header
      amTrace("Writing NKT file to: %s\n",pOutFileName);
      Nkt_CreateHeader(&nktHead, pSeq, bCompress);

      file = fopen(pOutFileName, "wb");
      bytes_written+=fwrite(&nktHead, sizeof(sNktHd), 1, file);
   }

// process tracks
    switch(pSeq->seqType){
    case ST_SINGLE:
        // handle single track sequence
        handleSingleTrack(pSeq, bCompress,&file,&blocks_written,&bytes_written);
        nktHead.NbOfBlocks=blocks_written;

    break;
    case ST_MULTI:
        // handle multi track sequence
        handleMultiTrack(pSeq, bCompress,&file,&blocks_written,&bytes_written);
        nktHead.NbOfBlocks=blocks_written;
    break;

    case ST_MULTI_SUB:
    default:
       error=TRUE;
    break;
    };


    if(file){
       fseek(file, 0, SEEK_SET);
       // update header
       fwrite(&nktHead, sizeof(sNktHd), 1, file);

       fclose(file); file=0;
       amTrace("Stored %d event blocks.\n",nktHead.NbOfBlocks);
     }

 if(error!=FALSE) return -1;

 return 0;
}

