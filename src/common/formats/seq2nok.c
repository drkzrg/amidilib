
// converts sequence to custom nok, binary format

#include "nok.h"
#include "midi_cmd.h"

#define NOKHEADERSIZE 10

void Nok_CreateHeader(sNokHd* header, const sSequence_t *pSeqData, const BOOL bCompress){
    WriteInt(&header->id,ID_NOK);
    WriteInt(&header->NbOfBlocks, 0);
    WriteShort(&header->division, pSeqData->timeDivision);
    WriteShort(&header->bPacked, bCompress);
    WriteShort(&header->version, 1);
}

static U8 *processSeqEvent(sEventList *pCurEvent,U8 *in, sNokBlock_t *nokBlock){
    // write data to out data block

    // increase nokBlock->blockSize after each data dump;
    // write to pData+nokBlock->blockSize;

    switch(pCurEvent->eventBlock.type){
        case T_NOTEON:{
            // dump data
            sNoteOn_EventBlock_t *pEventBlk=(sNoteOn_EventBlock_t *)&(pCurEvent->eventBlock);
            //write to output buffer
            note_on(pEventBlk->ubChannelNb,pEventBlk->eventData.noteNb,pEventBlk->eventData.velocity);
        } break;
        case T_NOTEOFF:{
            // dump data
            sNoteOff_EventBlock_t *pEventBlk=(sNoteOff_EventBlock_t *)&(pCurEvent->eventBlock);
            note_off(pEventBlk->ubChannelNb,pEventBlk->eventData.noteNb,pEventBlk->eventData.velocity);
        } break;
        case T_NOTEAFT:{
            // dump data
            sNoteAft_EventBlock_t *pEventBlk=(sNoteAft_EventBlock_t *)&(pCurEvent->eventBlock);
            copy_polyphonic_key_press(pEventBlk->ubChannelNb,pEventBlk->eventData.noteNb,pEventBlk->eventData.pressure);
        } break;
        case T_CONTROL:{
            // program change (dynamic according to connected device)
            sController_EventBlock_t *pEventBlk=(sController_EventBlock_t *)&(pCurEvent->eventBlock);
            copy_control_change(pEventBlk->eventData.controllerNb, pEventBlk->ubChannelNb, pEventBlk->eventData.value,0x00);
        } break;
        case T_PRG_CH:{
        // program change (dynamic according to connected device)
            sPrgChng_EventBlock_t *pEventBlk=(sPrgChng_EventBlock_t *)&(pCurEvent->eventBlock);
            copy_program_change(pEventBlk->ubChannelNb,pEventBlk->eventData.programNb);
        } break;
        case T_CHAN_AFT:{
            // dump data
            sChannelAft_EventBlock_t *pEventBlk=(sChannelAft_EventBlock_t *)&(pCurEvent->eventBlock);
            copy_channel_pressure(pEventBlk->ubChannelNb,pEventBlk->eventData.pressure);
        } break;
        case T_PITCH_BEND:{
            // dump data
            sPitchBend_EventBlock_t  *pEventBlk=(sPitchBend_EventBlock_t *)&(pCurEvent->eventBlock);
            copy_pitch_bend_2(pEventBlk->ubChannelNb,pEventBlk->eventData.LSB,pEventBlk->eventData.MSB);
        } break;
        case T_META_SET_TEMPO:{
            //set tempo
            sTempo_EventBlock_t  *pEventBlk=(sTempo_EventBlock_t  *)&(pCurEvent->eventBlock);
            //TODO create / add tempo block
        } break;
        case T_META_EOT:{
             //TODO create / add EOT block
            sEot_EventBlock_t *pEventBlk=(sEot_EventBlock_t *)&(pCurEvent->eventBlock);
        } break;
        case T_META_CUEPOINT:{
            //skip
            sCuePoint_EventBlock_t *pEventBlk=(sCuePoint_EventBlock_t *)&(pCurEvent->eventBlock);
        } break;
        case T_META_MARKER:{
            //skip
            sMarker_EventBlock_t *pEventBlk=(sMarker_EventBlock_t *)&(pCurEvent->eventBlock);
        } break;
        case T_META_SET_SIGNATURE:{
        //skip
            sTimeSignature_EventBlock_t *pEventBlk=(sTimeSignature_EventBlock_t *)&(pCurEvent->eventBlock);
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
        return -1;
        break;
  };
};

static S32 handleSingleTrack(const sSequence_t *pSeq, U8 *out,const BOOL bCompress){

    sTrack_t *pTrack=pSeq->arTracks[0];
    sEventList *eventPtr=pTrack->pTrkEventList;
    sNokBlock_t tempBlock;

    U32 currDelta=0;

    while(eventPtr!=NULL){
        // dump midi event block

        // dump data
        if(eventPtr!=NULL&&eventPtr->eventBlock.uiDeltaTime==currDelta){
            // store curent delta
            tempBlock.delta=currDelta;
            out=processSeqEvent(eventPtr,out, &tempBlock);

            // next event
            eventPtr=eventPtr->pNext;

            //check next events, dump until delta != 0
            while(eventPtr!=NULL&&eventPtr->eventBlock.uiDeltaTime==0){
                out=processSeqEvent(eventPtr,out,&tempBlock);

                // next event
                eventPtr=eventPtr->pNext;
            };

            if(eventPtr) currDelta=eventPtr->eventBlock.uiDeltaTime; //get next delta

        };



    };
 return 0;
}

static S32 handleMultiTrack(const sSequence_t *pSeq, U8 *out,const BOOL bCompress){

 return 0;
}

//converts sequence to nok file, optionally writes file named out
S32 Seq2Nok(const sSequence_t *pSeq, U8* out, const U8 *pOutFileName, const BOOL bCompress){
U32 bytes_written = 0;
sNokHd nktHead;

// create file header
   Nok_CreateHeader(&nktHead, pSeq, bCompress);

   UpdateBytesWritten(&bytes_written, NOKHEADERSIZE, -1);
   amMemCpy(out, &nktHead, NOKHEADERSIZE);      // cannot use sizeof(packs it to 16 bytes)
   out += NOKHEADERSIZE;

// process tracks
    switch(pSeq->seqType){
    case ST_SINGLE:
        //handle single track sequence
        handleSingleTrack(pSeq, out, bCompress);
    break;
    case ST_MULTI:
        //handle multi track sequence
        handleMultiTrack(pSeq, out, bCompress);
    break;

    case ST_MULTI_SUB:
    default:
        return -1;
        break;
    };


    if(pOutFileName){
       amTrace("Writing NOK file to: %s\n",pOutFileName);
       FILE* file = fopen(pOutFileName, "wb");
       // fwrite(midiTrackHeaderOut - sizeof(sNokHd), bytes_written, 1, file);
       fclose(file);
       amTrace("Written %d bytes\n",bytes_written);
     }

 return 0;
}

