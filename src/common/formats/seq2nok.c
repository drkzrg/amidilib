
// converts sequence to custom nok, binary format

#include "nok.h"

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
        } break;
        case T_NOTEOFF:{
        // dump data
        } break;
        case T_NOTEAFT:{
        // dump data
        } break;
        case T_CONTROL:{
        // program change (dynamic according to connected device)
        } break;
        case T_PRG_CH:{
        // program change (dynamic according to connected device)
        } break;
        case T_CHAN_AFT:{
        // dump data
        } break;
        case T_PITCH_BEND:{
        // dump data
        } break;
        case T_META_SET_TEMPO:{
        //set tempo
        } break;
        case T_META_EOT:{
        //write EOT
        } break;
        case T_META_CUEPOINT:{
        //skip
        } break;
        case T_META_MARKER:{
        //skip
        } break;
        case T_META_SET_SIGNATURE:{
        //skip
        } break;
        case T_SYSEX:{
        // copy data
        // format depends on connected device
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

