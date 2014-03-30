
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

static void processSeqEvent(sEventList *pCurEvent){
    // write data to out data block

    switch(pCurEvent->eventBlock.type){
        case T_NOTEON:{
            // dump data
            sNoteOn_EventBlock_t *pEventBlk=(sNoteOn_EventBlock_t *)&(pCurEvent->eventBlock);
            // write to output buffer
            copy_note_on(pEventBlk->ubChannelNb,pEventBlk->eventData.noteNb,pEventBlk->eventData.velocity);
            return;
        } break;
        case T_NOTEOFF:{
            // dump data
            sNoteOff_EventBlock_t *pEventBlk=(sNoteOff_EventBlock_t *)&(pCurEvent->eventBlock);
            copy_note_off(pEventBlk->ubChannelNb,pEventBlk->eventData.noteNb,pEventBlk->eventData.velocity);
            return;
        } break;
        case T_NOTEAFT:{
            // dump data
            sNoteAft_EventBlock_t *pEventBlk=(sNoteAft_EventBlock_t *)&(pCurEvent->eventBlock);
            copy_polyphonic_key_press(pEventBlk->ubChannelNb,pEventBlk->eventData.noteNb,pEventBlk->eventData.pressure);
            return;
        } break;
        case T_CONTROL:{
            // program change (dynamic according to connected device)
            sController_EventBlock_t *pEventBlk=(sController_EventBlock_t *)&(pCurEvent->eventBlock);
            copy_control_change(pEventBlk->eventData.controllerNb, pEventBlk->ubChannelNb, pEventBlk->eventData.value,0x00);
            return;
        } break;
        case T_PRG_CH:{
        // program change (dynamic according to connected device)
            sPrgChng_EventBlock_t *pEventBlk=(sPrgChng_EventBlock_t *)&(pCurEvent->eventBlock);
            copy_program_change(pEventBlk->ubChannelNb,pEventBlk->eventData.programNb);
            return;
        } break;
        case T_CHAN_AFT:{
            // dump data
            sChannelAft_EventBlock_t *pEventBlk=(sChannelAft_EventBlock_t *)&(pCurEvent->eventBlock);
            copy_channel_pressure(pEventBlk->ubChannelNb,pEventBlk->eventData.pressure);
            return;
        } break;
        case T_PITCH_BEND:{
            // dump data
            sPitchBend_EventBlock_t  *pEventBlk=(sPitchBend_EventBlock_t *)&(pCurEvent->eventBlock);
            copy_pitch_bend_2(pEventBlk->ubChannelNb,pEventBlk->eventData.LSB,pEventBlk->eventData.MSB);
            return;
        } break;
        case T_META_SET_TEMPO:{
            //set tempo
            sTempo_EventBlock_t  *pEventBlk=(sTempo_EventBlock_t  *)&(pCurEvent->eventBlock);
            // TODO create / add tempo block
            return;
        } break;
        case T_META_EOT:{
             // TODO create / add EOT block
            sEot_EventBlock_t *pEventBlk=(sEot_EventBlock_t *)&(pCurEvent->eventBlock);
            return;
        } break;
        case T_META_CUEPOINT:{
            //skip
            sCuePoint_EventBlock_t *pEventBlk=(sCuePoint_EventBlock_t *)&(pCurEvent->eventBlock);
            return;
        } break;
        case T_META_MARKER:{
            //skip
            sMarker_EventBlock_t *pEventBlk=(sMarker_EventBlock_t *)&(pCurEvent->eventBlock);
            return;
        } break;
        case T_META_SET_SIGNATURE:{
        //skip
            sTimeSignature_EventBlock_t *pEventBlk=(sTimeSignature_EventBlock_t *)&(pCurEvent->eventBlock);
             return;
        } break;
        case T_SYSEX:{
        // copy data
        // format depends on connected device
         sSysEX_EventBlock_t *pEventBlk=(sSysEX_EventBlock_t *)&(pCurEvent->eventBlock);
          #ifdef DEBUG_BUILD
            amTrace((const U8*)"Copy SysEX Message.\n");
          #endif
         //TODO: check buffer overflow
          amMemCpy(&MIDIsendBuffer[MIDIbytesToSend],pEventBlk->pBuffer,pEventBlk->bufferSize);
          MIDIbytesToSend+=pEventBlk->bufferSize;
        } break;
    default:
        // error not handled
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

    sTrack_t *pTrack=pSeq->arTracks[0];
    sEventList *eventPtr=pTrack->pTrkEventList;
    U32 currDelta=0;
    sNktBlk stBlock;

    while(eventPtr!=NULL){

        // dump data
        if(eventPtr->eventBlock.uiDeltaTime==currDelta){
            stBlock.delta=currDelta;

           // if event is tempo related then create event with curent delta
           // and go to next event

            //skip uninteresting events
            while((eventPtr!=NULL) &&( (eventPtr->eventBlock.type==T_META_CUEPOINT) || (eventPtr->eventBlock.type==T_META_MARKER) )){
                printf("Skip event >> %d\n",eventPtr->eventBlock.type);
                eventPtr=eventPtr->pNext;
            }

            if((eventPtr!=NULL) &&(eventPtr->eventBlock.type==T_META_SET_TEMPO)){
                U32 tempo = ((sTempo_EventBlock_t *)(eventPtr->eventBlock.dataPtr))->eventData.tempoVal;
#ifdef DEBUG_BUILD
                printf("Write Set Tempo: %lu event\n",tempo);
#endif
                stBlock.delta=currDelta;
                stBlock.blockSize=sizeof(U32);
                stBlock.msgType=(U16)seq2nktMap[eventPtr->eventBlock.type];
                U8 *pData=0;

                pData=amMallocEx(stBlock.blockSize,PREFER_TT);

                if(pData!=NULL){
                    amMemCpy(pData,&tempo,stBlock.blockSize);

                    //write block to file
                    if(file!=NULL){
                        *bytesWritten+=fwrite(&stBlock,sizeof(stBlock),1,*file);
                        *bytesWritten+=fwrite(pData,sizeof(U32),1,*file);
                    }
                    ++(*blocksWritten);
#ifdef DEBUG_BUILD
                    printf("delta [%lu] type:[%d] size:[%u] bytes \n",stBlock.delta, stBlock.msgType, stBlock.blockSize );
#endif
                    amFree((void **)&pData);
                }

                eventPtr=eventPtr->pNext;
            }

            if((eventPtr!=NULL) &&(eventPtr->eventBlock.type==T_META_EOT)){
#ifdef DEBUG_BUILD
                printf("Write End of Track \n");
#endif

                stBlock.delta=currDelta;
                stBlock.blockSize=0;
                stBlock.msgType=(U16)seq2nktMap[eventPtr->eventBlock.type];

                //write block to file
                if(file!=NULL){
                    *bytesWritten+=fwrite(&stBlock,sizeof(stBlock),1,*file);
                    //no data to write
                }
                ++(*blocksWritten);
#ifdef DEBUG_BUILD
                printf("delta [%lu] type:[%d] size:[%u] bytes \n",stBlock.delta, stBlock.msgType, stBlock.blockSize );
#endif
                eventPtr=eventPtr->pNext;
            }

            if(eventPtr!=0){
                //process events as normal

                processSeqEvent(eventPtr);
                stBlock.delta = currDelta;
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
                         eventPtr=eventPtr->pNext;
                    }

                    if((eventPtr!=NULL) &&(eventPtr->eventBlock.type==T_META_EOT)){
                        printf("Write End of Track, event delta 0\n");
                        eventPtr=eventPtr->pNext;
                    }

                    if(eventPtr!=NULL) {
                        processSeqEvent(eventPtr);

                        // next event
                        eventPtr=eventPtr->pNext;
                    }

                };

                if(eventPtr) currDelta=eventPtr->eventBlock.uiDeltaTime; //get next delta

            }//end null check

            // dump midi event block to memory
            if(MIDIbytesToSend>0){
                U8 *pData=amMallocEx(MIDIbytesToSend,PREFER_TT);
                stBlock.blockSize=MIDIbytesToSend;

                if(pData!=NULL){
                    amMemCpy(pData,MIDIsendBuffer,MIDIbytesToSend);

                    //write block to file
                    if(file!=NULL){
                        *bytesWritten+=fwrite(&stBlock,sizeof(sNktBlk),1,*file);
                        *bytesWritten+=fwrite(pData,stBlock.blockSize,1,*file);
                    }
                    ++(*blocksWritten);
    #ifdef DEBUG_BUILD
                    printf("delta [%lu] type:[%d] size:[%u] bytes \n",stBlock.delta, stBlock.msgType, stBlock.blockSize);
    #endif
                    //clear buffer
                    amMemSet(MIDIsendBuffer,0,MIDI_SENDBUFFER_SIZE);
                    MIDIbytesToSend=0;

                    amFree((void **)&pData);
                }else{
                     printf("Error: Couldn't allocate memory for data \n");
                }
            }

        };



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

