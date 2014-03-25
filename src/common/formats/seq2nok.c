
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

static S32 handleSingleTrack(const sSequence_t *pSeq, U8 *out,const BOOL bCompress){

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

