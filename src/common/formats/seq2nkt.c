
// converts sequence to custom nok, binary format

#include "nkt.h"
#include "midi_cmd.h"

void Nkt_CreateHeader(sNktHd* header, const sSequence_t *pSeqData, const BOOL bCompress){
    WriteInt(&header->id,ID_NKT);
    WriteInt(&header->NbOfBlocks, 0);
    WriteShort(&header->division, pSeqData->timeDivision);
    WriteShort(&header->bPacked, bCompress);
    WriteShort(&header->version, 1);
}

static U8 *processSeqEvent(sEventList *pCurEvent,U8 *in, sNktBlock_t *nokBlock){
    // write data to out data block

    switch(pCurEvent->eventBlock.type){
        case T_NOTEON:{
            // dump data
            sNoteOn_EventBlock_t *pEventBlk=(sNoteOn_EventBlock_t *)&(pCurEvent->eventBlock);
            // write to output buffer
            note_on(pEventBlk->ubChannelNb,pEventBlk->eventData.noteNb,pEventBlk->eventData.velocity);
            nokBlock->blockSize+=sizeof(sNoteOn_EventBlock_t);
            return (in+sizeof(sNoteOn_EventBlock_t));

        } break;
        case T_NOTEOFF:{
            // dump data
            sNoteOff_EventBlock_t *pEventBlk=(sNoteOff_EventBlock_t *)&(pCurEvent->eventBlock);
            note_off(pEventBlk->ubChannelNb,pEventBlk->eventData.noteNb,pEventBlk->eventData.velocity);
            nokBlock->blockSize+=sizeof(sNoteOff_EventBlock_t);

            return (in+sizeof(sNoteOff_EventBlock_t));
        } break;
        case T_NOTEAFT:{
            // dump data
            sNoteAft_EventBlock_t *pEventBlk=(sNoteAft_EventBlock_t *)&(pCurEvent->eventBlock);
            copy_polyphonic_key_press(pEventBlk->ubChannelNb,pEventBlk->eventData.noteNb,pEventBlk->eventData.pressure);
            nokBlock->blockSize+=sizeof(sNoteAft_EventBlock_t);

            return (in+sizeof(sNoteAft_EventBlock_t));

        } break;
        case T_CONTROL:{
            // program change (dynamic according to connected device)
            sController_EventBlock_t *pEventBlk=(sController_EventBlock_t *)&(pCurEvent->eventBlock);
            copy_control_change(pEventBlk->eventData.controllerNb, pEventBlk->ubChannelNb, pEventBlk->eventData.value,0x00);
            nokBlock->blockSize+=sizeof(sController_EventBlock_t);
            return (in+sizeof(sController_EventBlock_t));
        } break;
        case T_PRG_CH:{
        // program change (dynamic according to connected device)
            sPrgChng_EventBlock_t *pEventBlk=(sPrgChng_EventBlock_t *)&(pCurEvent->eventBlock);
            copy_program_change(pEventBlk->ubChannelNb,pEventBlk->eventData.programNb);
            nokBlock->blockSize+=sizeof(sPrgChng_EventBlock_t);
            return (in+sizeof(sPrgChng_EventBlock_t));
        } break;
        case T_CHAN_AFT:{
            // dump data
            sChannelAft_EventBlock_t *pEventBlk=(sChannelAft_EventBlock_t *)&(pCurEvent->eventBlock);
            copy_channel_pressure(pEventBlk->ubChannelNb,pEventBlk->eventData.pressure);
            nokBlock->blockSize+=sizeof(sChannelAft_EventBlock_t);
            return (in+sizeof(sChannelAft_EventBlock_t));
        } break;
        case T_PITCH_BEND:{
            // dump data
            sPitchBend_EventBlock_t  *pEventBlk=(sPitchBend_EventBlock_t *)&(pCurEvent->eventBlock);
            copy_pitch_bend_2(pEventBlk->ubChannelNb,pEventBlk->eventData.LSB,pEventBlk->eventData.MSB);
            nokBlock->blockSize+=sizeof(sPitchBend_EventBlock_t);
            return (in+sizeof(sPitchBend_EventBlock_t));
        } break;
        case T_META_SET_TEMPO:{
            //set tempo
            sTempo_EventBlock_t  *pEventBlk=(sTempo_EventBlock_t  *)&(pCurEvent->eventBlock);
            // TODO create / add tempo block
            return in;
        } break;
        case T_META_EOT:{
             // TODO create / add EOT block
            sEot_EventBlock_t *pEventBlk=(sEot_EventBlock_t *)&(pCurEvent->eventBlock);
            return (in);
        } break;
        case T_META_CUEPOINT:{
            //skip
            sCuePoint_EventBlock_t *pEventBlk=(sCuePoint_EventBlock_t *)&(pCurEvent->eventBlock);
            return in;
        } break;
        case T_META_MARKER:{
            //skip
            sMarker_EventBlock_t *pEventBlk=(sMarker_EventBlock_t *)&(pCurEvent->eventBlock);
            return in;
        } break;
        case T_META_SET_SIGNATURE:{
        //skip
            sTimeSignature_EventBlock_t *pEventBlk=(sTimeSignature_EventBlock_t *)&(pCurEvent->eventBlock);
             return in;
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
        //error not handled
        return NULL;
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


static S32 handleSingleTrack(const sSequence_t *pSeq, U8 *out,const BOOL bCompress, FILE **file, U32 *blocksWritten){

    printf("Processing single track\n");

    sTrack_t *pTrack=pSeq->arTracks[0];
    sEventList *eventPtr=pTrack->pTrkEventList;
    sNktBlock_t tempBlock;

    amMemSet(&tempBlock,0,sizeof(sNktBlock_t));

    U32 currDelta=0;

    while(eventPtr!=NULL){

        // dump data
        if(eventPtr!=NULL&&eventPtr->eventBlock.uiDeltaTime==currDelta){
            // store curent delta
            tempBlock.delta=currDelta; //save delta
            tempBlock.blockSize=0;   //reset block size
            tempBlock.msgType=0;
            tempBlock.pData=0;

            // if event is tempo related then create event with curent delta
            // and go to next event

            //skip uninteresting events
            while((eventPtr!=NULL) &&( (eventPtr->eventBlock.type==T_META_CUEPOINT) || (eventPtr->eventBlock.type==T_META_MARKER) )){
                printf("Skip event >> %d\n",eventPtr->eventBlock.type);
                eventPtr=eventPtr->pNext;
            }

            if((eventPtr!=NULL) &&(eventPtr->eventBlock.type==T_META_SET_TEMPO)){
                printf("Write Set Tempo: %lu event\n",((sTempo_EventBlock_t *)(eventPtr->eventBlock.dataPtr))->eventData.tempoVal);
                 eventPtr=eventPtr->pNext;
            }

            if((eventPtr!=NULL) &&(eventPtr->eventBlock.type==T_META_EOT)){
                printf("Write End of Track \n");
                eventPtr=eventPtr->pNext;
            }

            if(eventPtr!=0){
                //process events as normal
                tempBlock.msgType=(eNktMsgType)seq2nktMap[eventPtr->eventBlock.type];
                out=processSeqEvent(eventPtr,out, &tempBlock);


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

                    if(eventPtr!=NULL) {out=processSeqEvent(eventPtr,out,&tempBlock);
                        // next event
                        eventPtr=eventPtr->pNext;
                    }

                };

                if(eventPtr) currDelta=eventPtr->eventBlock.uiDeltaTime; //get next delta

            }//end null check
        };

        // dump midi event block to memory
        if(MIDIbytesToSend>0){
            tempBlock.pData=amMallocEx(MIDIbytesToSend,PREFER_TT);

            if(tempBlock.pData!=NULL){
                amMemCpy(tempBlock.pData,MIDIsendBuffer,MIDIbytesToSend);
            }

            // copy data to our internal buffer
            out=WriteInt(out,tempBlock.delta);
            out=WriteShort(out,(U16)tempBlock.msgType);
            out=WriteInt(out,tempBlock.blockSize);
            amMemCpy(out,tempBlock.pData,tempBlock.blockSize*sizeof(U8));

            //write block to file
            if(file!=NULL){
                fwrite(&tempBlock.delta,sizeof(tempBlock.delta),1,*file);
                fwrite(&tempBlock.msgType,sizeof(tempBlock.msgType),1,*file);
                fwrite(&tempBlock.blockSize,sizeof(tempBlock.blockSize),1,*file);
                fwrite(&tempBlock.pData,sizeof(tempBlock.blockSize*sizeof(U8)),1,*file);
            }
            ++(*blocksWritten);

            printf("delta [%lu] type:[%d] size:[%lu] bytes \n",tempBlock.delta, tempBlock.msgType, tempBlock.blockSize );

            //clear buffer
            amMemSet(MIDIsendBuffer,0,MIDI_BUFFER_SIZE);
            MIDIbytesToSend=0;
        }

    };

    printf("Event blocks written: %lu\n",*blocksWritten);
 return 0;
}

static S32 handleMultiTrack(const sSequence_t *pSeq, U8 *out,const BOOL bCompress, FILE **file, U32 *blocksWritten){

 return 0;
}

S32 writeNktBlockToFile(){

}

//converts sequence to nkt file, optionally writes file named out
S32 Seq2Nkt(const sSequence_t *pSeq, U8* out, const U8 *pOutFileName, const BOOL bCompress){
U32 bytes_written = 0;
U32 blocks_written = 0;

sNktHd nktHead;
U8 *pTemp=out;
BOOL error=FALSE;
FILE* file=0;

// create file header
   Nkt_CreateHeader(&nktHead, pSeq, bCompress);

   UpdateBytesWritten(&bytes_written, sizeof(sNktHd), -1);
   amMemCpy(pTemp, &nktHead, sizeof(sNktHd));
   pTemp += sizeof(sNktHd);

   if(pOutFileName){
      amTrace("Writing NKT file to: %s\n",pOutFileName);
      file = fopen(pOutFileName, "wb");
      fwrite(out, bytes_written, 1, file);
   }

// process tracks
    switch(pSeq->seqType){
    case ST_SINGLE:
        // handle single track sequence
        handleSingleTrack(pSeq, pTemp, bCompress,&file,&blocks_written);
        nktHead.NbOfBlocks=blocks_written;

    break;
    case ST_MULTI:
        // handle multi track sequence
        handleMultiTrack(pSeq, pTemp, bCompress,&file,&blocks_written);
        nktHead.NbOfBlocks=blocks_written;
    break;

    case ST_MULTI_SUB:
    default:
       error=TRUE;
    break;
    };

    if(pOutFileName){
       fclose(file);
       amTrace("Written %d bytes\n",bytes_written);
     }

 if(error!=FALSE) return -1;

 return 0;
}

