
// converts sequence to custom nkt, binary format

/**  Copyright 2007-2014 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

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

void processSeqEvent(sEventList *pCurEvent, U8 *tab,U32 *bufPos, U32 *bufDataSize){
    // write data to out data block

    switch(pCurEvent->eventBlock.type){
        case T_NOTEON:{
            // dump data

            sNoteOn_EventBlock_t *pEventBlk=(sNoteOn_EventBlock_t *)(pCurEvent->eventBlock.dataPtr);
            amTrace("T_NOTEON ch: %d n: %d vel:%d \n",pEventBlk->ubChannelNb, pEventBlk->eventData.noteNb, pEventBlk->eventData.velocity);
            // write to output buffer
            *(tab + (*bufPos))= (U8)(EV_NOTE_ON<<4)|pEventBlk->ubChannelNb; (*bufPos)++;
          //  tab[]=(U8)(EV_NOTE_ON<<4)|pEventBlk->ubChannelNb; (*bufPos)++;
            tab[(*bufPos)]=pEventBlk->eventData.noteNb; (*bufPos)++;
            tab[(*bufPos)]=pEventBlk->eventData.velocity; (*bufPos)++;
            (*bufDataSize)+=3;
            return;
        } break;
        case T_NOTEOFF:{
            // dump data
            sNoteOff_EventBlock_t *pEventBlk=(sNoteOff_EventBlock_t *)pCurEvent->eventBlock.dataPtr;
            amTrace("T_NOTEOFF ch: %d n: %d vel:%d \n",pEventBlk->ubChannelNb, pEventBlk->eventData.noteNb, pEventBlk->eventData.velocity);
            tab[(*bufPos)]=(U8)(EV_NOTE_OFF<<4)|(pEventBlk->ubChannelNb); (*bufPos)++;
            tab[(*bufPos)]=pEventBlk->eventData.noteNb; (*bufPos)++;
            tab[(*bufPos)]=pEventBlk->eventData.velocity; (*bufPos)++;
            (*bufDataSize)+=3;
            return;
        } break;
        case T_NOTEAFT:{
            // dump data
            amTrace("T_NOTEAFT \n");
            sNoteAft_EventBlock_t *pEventBlk=(sNoteAft_EventBlock_t *)pCurEvent->eventBlock.dataPtr;
            tab[(*bufPos)]=(EV_NOTE_AFTERTOUCH<<4)|pEventBlk->ubChannelNb; (*bufPos)++;
            tab[(*bufPos)]=pEventBlk->eventData.noteNb; (*bufPos)++;
            tab[(*bufPos)]=pEventBlk->eventData.pressure; (*bufPos)++;
            (*bufDataSize)+=3;

            return;
        } break;
        case T_CONTROL:{
            amTrace("T_CONTROL \n");
            // program change (dynamic according to connected device)
            sController_EventBlock_t *pEventBlk=(sController_EventBlock_t *)pCurEvent->eventBlock.dataPtr;

            tab[(*bufPos)]=(EV_CONTROLLER<<4)|pEventBlk->ubChannelNb; (*bufPos)++;
            tab[(*bufPos)]=pEventBlk->eventData.controllerNb; (*bufPos)++;
            tab[(*bufPos)]=pEventBlk->eventData.value; (*bufPos)++;
            tab[(*bufPos)]=0x00; (*bufPos)++;
            (*bufDataSize)+=4;
            return;
        } break;
        case T_PRG_CH:{
        // program change (dynamic according to connected device)
         amTrace("T_PRG_CH \n");
            sPrgChng_EventBlock_t *pEventBlk=(sPrgChng_EventBlock_t *)pCurEvent->eventBlock.dataPtr;
            tab[(*bufPos)]=(EV_PROGRAM_CHANGE<<4)|pEventBlk->ubChannelNb; (*bufPos)++;
            tab[(*bufPos)]=pEventBlk->eventData.programNb; (*bufPos)++;
            (*bufDataSize)+=2;
            return;
        } break;
        case T_CHAN_AFT:{
            // dump data
         amTrace("T_CHAN_AFT \n");
            sChannelAft_EventBlock_t *pEventBlk=(sChannelAft_EventBlock_t *)pCurEvent->eventBlock.dataPtr;
            tab[(*bufPos)] = (EV_CHANNEL_AFTERTOUCH<<4) | pEventBlk->ubChannelNb; (*bufPos)++;
            tab[(*bufPos)] = pEventBlk->eventData.pressure; (*bufPos)++;
            (*bufDataSize)+=2;
            return;
        } break;
        case T_PITCH_BEND:{
            // dump data
         amTrace("T_PITCH_BEND \n");
            sPitchBend_EventBlock_t  *pEventBlk=(sPitchBend_EventBlock_t *)pCurEvent->eventBlock.dataPtr;

            tab[(*bufPos)]=(EV_PITCH_BEND<<4)|pEventBlk->ubChannelNb; (*bufPos)++;
            tab[(*bufPos)]=pEventBlk->eventData.LSB; (*bufPos)++; //LSB
            tab[(*bufPos)]=pEventBlk->eventData.MSB; (*bufPos)++; //MSB
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
          sSysEX_EventBlock_t *pEventBlk=(sSysEX_EventBlock_t *)pCurEvent->eventBlock.dataPtr;
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
    NKT_MIDIDATA        //	T_SYSEX,
};


#ifdef ENABLE_GEMDOS_IO
static S32 handleSingleTrack(const sSequence_t *pSeq, const BOOL bCompress, S32 fh,U32 *blocksWritten, U32 *bytesWritten)
#else
static S32 handleSingleTrack(const sSequence_t *pSeq, const BOOL bCompress, FILE **file,U32 *blocksWritten, U32 *bytesWritten)
#endif
{

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

            //skip uninteresting events
            while((eventPtr!=NULL) &&( (eventPtr->eventBlock.type==T_META_CUEPOINT) || (eventPtr->eventBlock.type==T_META_MARKER) )){
                printf("Skip event >> %d\n",eventPtr->eventBlock.type);
                eventPtr=eventPtr->pNext;
            }

            if((eventPtr!=NULL) &&(eventPtr->eventBlock.type==T_META_SET_TEMPO)){
                U32 tempo = ((sTempo_EventBlock_t *)(eventPtr->eventBlock.dataPtr))->eventData.tempoVal;

                amTrace("Write Set Tempo: %lu event\n",tempo);

                stBlock.blockSize=sizeof(U32);
                stBlock.msgType=(U16)seq2nktMap[eventPtr->eventBlock.type];

                //write block to file
                U32 VLQdeltaTemp=0;

                // write VLQ delta
                S32 count=0;
                count=WriteVarLen((S32)currDelta, (U8 *)&VLQdeltaTemp);

#ifdef ENABLE_GEMDOS_IO
                *bytesWritten+=Fwrite(fh,count,&VLQdeltaTemp);
                *bytesWritten+=Fwrite(fh,sizeof(stBlock),&stBlock);
                *bytesWritten+=Fwrite(fh,sizeof(U32),&tempo);
#else
                *bytesWritten+=fwrite(&VLQdeltaTemp,count,1,*file);
                *bytesWritten+=fwrite(&stBlock,sizeof(stBlock),1,*file);
                *bytesWritten+=fwrite(&tempo,sizeof(U32),1,*file);
#endif

                ++(*blocksWritten);

                amTrace("delta [%lu] type:[%d] size:[%u] bytes \n",currDelta, stBlock.msgType, stBlock.blockSize );
                eventPtr=eventPtr->pNext;
            }

            if((eventPtr!=NULL) &&(eventPtr->eventBlock.type==T_META_EOT)){

                amTrace("Write End of Track \n");
                stBlock.blockSize=0;
                stBlock.msgType=(U16)seq2nktMap[eventPtr->eventBlock.type];

                // write block to file
                // write VLQ delta
                U32 VLQdeltaTemp=0;
                S32 count=0;
                count=WriteVarLen((S32)currDelta, (U8 *)&VLQdeltaTemp);
#ifdef ENABLE_GEMDOS_IO
                *bytesWritten+=Fwrite(fh,count,&VLQdeltaTemp);
                *bytesWritten+=Fwrite(fh,sizeof(stBlock),&stBlock);
#else
                *bytesWritten+=fwrite(&VLQdeltaTemp,count,1,*file);
                *bytesWritten+=fwrite(&stBlock,sizeof(stBlock),1,*file);
#endif

                //no data to write
                ++(*blocksWritten);
                eventPtr=eventPtr->pNext;
            }

            if(eventPtr!=0){

                // process events as normal
                processSeqEvent(eventPtr, &tempBuffer[0], &bufPos, &bufDataSize);
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

                        stBlock.blockSize=sizeof(U32);
                        stBlock.msgType=(U16)seq2nktMap[eventPtr->eventBlock.type];

                        //write block to file

                        // write VLQ delta
                        S32 count=0;
                        U32 VLQdeltaTemp=0;

                        count=WriteVarLen((S32)currDelta, (U8 *)&VLQdeltaTemp);
#ifdef ENABLE_GEMDOS_IO
                        *bytesWritten+=Fwrite(fh, count, &VLQdeltaTemp);
                        *bytesWritten+=Fwrite(fh, sizeof(stBlock),&stBlock);
                        *bytesWritten+=Fwrite(fh, sizeof(U32),&tempo);
#else
                        *bytesWritten+=fwrite(&VLQdeltaTemp,count,1,*file);
                        *bytesWritten+=fwrite(&stBlock,sizeof(stBlock),1,*file);
                        *bytesWritten+=fwrite(&tempo,sizeof(U32),1,*file);
#endif


                        ++(*blocksWritten);

                        amTrace("delta [%lu] type:[%d] size:[%u] bytes \n",currDelta, stBlock.msgType, stBlock.blockSize );
                        eventPtr=eventPtr->pNext;
                    }

                    if((eventPtr!=NULL) &&(eventPtr->eventBlock.type==T_META_EOT)){
                        printf("Write End of Track, event delta 0\n");

                        stBlock.blockSize=0;
                        stBlock.msgType=(U16)seq2nktMap[eventPtr->eventBlock.type];

                        // write block to file
                        // write VLQ data
                        U32 VLQdeltaTemp=0;
                        S32 count=0;
                        count=WriteVarLen((S32)currDelta, (U8 *)&VLQdeltaTemp);

#ifdef ENABLE_GEMDOS_IO
                        *bytesWritten+=Fwrite(fh, count, &VLQdeltaTemp);
                        *bytesWritten+=Fwrite(fh, sizeof(stBlock), &stBlock);
#else
                        *bytesWritten+=fwrite(&VLQdeltaTemp,count,1,*file);
                        *bytesWritten+=fwrite(&stBlock,sizeof(stBlock),1,*file);
#endif

                        //no data to write
                        ++(*blocksWritten);

                        eventPtr=eventPtr->pNext;
                    }

                    if(eventPtr!=NULL) {
                        processSeqEvent(eventPtr, &tempBuffer[0], &bufPos, &bufDataSize);

                        // next event
                        eventPtr=eventPtr->pNext;
                    }

                }; //end delta == 0 if


                // dump midi event block to memory
                if(bufPos>0){
                    stBlock.blockSize=bufDataSize;

                    amTrace("[DATA] ");

                        for(int j=0;j<bufDataSize;j++){
                            amTrace("0x%x ",tempBuffer[j]);
                        }
                    amTrace(" [/DATA]\n");

                    //write block to file

                    // write VLQ delta
                    S32 count=0;
                    U32 VLQdeltaTemp=0;

                    count=WriteVarLen((S32)currDelta, (U8 *)&VLQdeltaTemp);
                    amTrace("Write block size %d\n",stBlock.blockSize);

#ifdef ENABLE_GEMDOS_IO
                    *bytesWritten+=Fwrite(fh, count, &VLQdeltaTemp);
                    *bytesWritten+=Fwrite(fh, sizeof(sNktBlk), &stBlock);
                    *bytesWritten+=Fwrite(fh, stBlock.blockSize, &tempBuffer[0]);
#else
                    *bytesWritten+=fwrite(&VLQdeltaTemp,count,1,*file);
                    *bytesWritten+=fwrite(&stBlock, sizeof(sNktBlk), 1, *file);
                    *bytesWritten+=fwrite(&tempBuffer[0],stBlock.blockSize,1,*file);
#endif


                    ++(*blocksWritten);

                    amTrace("delta [%lu] type:[%d] size:[%u] bytes \n",currDelta, stBlock.msgType, stBlock.blockSize);

                    //clear buffer
                    bufDataSize=0;
                    bufPos=0;
                    amMemSet(tempBuffer,0,32 * 1024);

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

#ifdef ENABLE_GEMDOS_IO
static S32 handleMultiTrack(const sSequence_t *pSeq, const BOOL bCompress, S32 fh, U32 *bytesWritten, U32 *blocksWritten){
#else
static S32 handleMultiTrack(const sSequence_t *pSeq, const BOOL bCompress, FILE **file, U32 *bytesWritten, U32 *blocksWritten){
#endif
/*
//TODO
    printf("Processing multi track sequence\n");
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

            //skip uninteresting events
            while((eventPtr!=NULL) &&( (eventPtr->eventBlock.type==T_META_CUEPOINT) || (eventPtr->eventBlock.type==T_META_MARKER) )){
                printf("Skip event >> %d\n",eventPtr->eventBlock.type);
                eventPtr=eventPtr->pNext;
            }

            if((eventPtr!=NULL) &&(eventPtr->eventBlock.type==T_META_SET_TEMPO)){
                U32 tempo = ((sTempo_EventBlock_t *)(eventPtr->eventBlock.dataPtr))->eventData.tempoVal;

                amTrace("Write Set Tempo: %lu event\n",tempo);

                stBlock.blockSize=sizeof(U32);
                stBlock.msgType=(U16)seq2nktMap[eventPtr->eventBlock.type];

                //write block to file
                U32 VLQdeltaTemp=0;
                if(file!=NULL){

                   // write VLQ delta
                   S32 count=0;
                   count=WriteVarLen((S32)currDelta, (U8 *)&VLQdeltaTemp);
                   *bytesWritten+=fwrite(&VLQdeltaTemp,count,1,*file);

                   *bytesWritten+=fwrite(&stBlock,sizeof(stBlock),1,*file);
                   *bytesWritten+=fwrite(&tempo,sizeof(U32),1,*file);
                 }
                ++(*blocksWritten);

                amTrace("delta [%lu] type:[%d] size:[%u] bytes \n",currDelta, stBlock.msgType, stBlock.blockSize );
                eventPtr=eventPtr->pNext;
            }

            if((eventPtr!=NULL) &&(eventPtr->eventBlock.type==T_META_EOT)){

                amTrace("Write End of Track \n");
                stBlock.blockSize=0;
                stBlock.msgType=(U16)seq2nktMap[eventPtr->eventBlock.type];

                //write block to file
                if(file!=NULL){
                    // write VLQ delta
                    U32 VLQdeltaTemp=0;
                    S32 count=0;
                    count=WriteVarLen((S32)currDelta, (U8 *)&VLQdeltaTemp);
                    *bytesWritten+=fwrite(&VLQdeltaTemp,count,1,*file);

                    *bytesWritten+=fwrite(&stBlock,sizeof(stBlock),1,*file);
                    //no data to write
                }
                ++(*blocksWritten);
                eventPtr=eventPtr->pNext;

            }

            if(eventPtr!=0){

                // process events as normal
                processSeqEvent(eventPtr, &tempBuffer[0], &bufPos, &bufDataSize);
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

                        stBlock.blockSize=sizeof(U32);
                        stBlock.msgType=(U16)seq2nktMap[eventPtr->eventBlock.type];

                        //write block to file

                        if(file!=NULL){
                           // write VLQ delta
                           S32 count=0;
                           U32 VLQdeltaTemp=0;

                           count=WriteVarLen((S32)currDelta, (U8 *)&VLQdeltaTemp);
                           *bytesWritten+=fwrite(&VLQdeltaTemp,count,1,*file);

                           *bytesWritten+=fwrite(&stBlock,sizeof(stBlock),1,*file);
                           *bytesWritten+=fwrite(&tempo,sizeof(U32),1,*file);
                         }
                        ++(*blocksWritten);

                        amTrace("delta [%lu] type:[%d] size:[%u] bytes \n",currDelta, stBlock.msgType, stBlock.blockSize );
                        eventPtr=eventPtr->pNext;
                    }

                    if((eventPtr!=NULL) &&(eventPtr->eventBlock.type==T_META_EOT)){
                        printf("Write End of Track, event delta 0\n");

                        stBlock.blockSize=0;
                        stBlock.msgType=(U16)seq2nktMap[eventPtr->eventBlock.type];

                        //write block to file
                        if(file!=NULL){
                            // write VLQ data
                            U32 VLQdeltaTemp=0;
                            S32 count=0;
                            count=WriteVarLen((S32)currDelta, (U8 *)&VLQdeltaTemp);

                            *bytesWritten+=fwrite(&VLQdeltaTemp,count,1,*file);
                            *bytesWritten+=fwrite(&stBlock,sizeof(stBlock),1,*file);
                            //no data to write
                        }
                        ++(*blocksWritten);

                        eventPtr=eventPtr->pNext;
                    }

                    if(eventPtr!=NULL) {
                        processSeqEvent(eventPtr, &tempBuffer[0], &bufPos, &bufDataSize);

                        // next event
                        eventPtr=eventPtr->pNext;
                    }

                }; //end delta == 0 while


                // dump midi event block to memory
                if(bufPos>0){
                    stBlock.blockSize=bufDataSize;

                    amTrace("[DATA] ");

                        for(int j=0;j<bufDataSize;j++){
                            amTrace("0x%x ",tempBuffer[j]);
                        }
                    amTrace(" [/DATA]\n");

                    //write block to file

                    if(file!=NULL){
                      // write VLQ delta
                      S32 count=0;
                      U32 VLQdeltaTemp=0;

                      count=WriteVarLen((S32)currDelta, (U8 *)&VLQdeltaTemp);
                      *bytesWritten+=fwrite(&VLQdeltaTemp,count,1,*file);

                      amTrace("Write block size %d\n",stBlock.blockSize);
                      *bytesWritten+=fwrite(&stBlock, sizeof(sNktBlk), 1, *file);
                      *bytesWritten+=fwrite(&tempBuffer[0],stBlock.blockSize,1,*file);
                    }
                    ++(*blocksWritten);

                    amTrace("delta [%lu] type:[%d] size:[%u] bytes \n",currDelta, stBlock.msgType, stBlock.blockSize);

                    //clear buffer
                    bufDataSize=0;
                    bufPos=0;
                    amMemSet(tempBuffer,0,32 * 1024);

                }


                if(eventPtr) {
                    currDelta=eventPtr->eventBlock.uiDeltaTime; //get next delta
                    amTrace("Next delta: %lu\n",currDelta);
                }

            }//end null check

        }; // end while

    }; //end while end of sequence
    printf("Event blocks written: %lu, total bytes of data written %lu\n",*blocksWritten,*bytesWritten);
*/
 return 0;
}


//converts sequence to nkt file, optionally writes file named out
S32 Seq2NktFile(const sSequence_t *pSeq, const U8 *pOutFileName, const BOOL bCompress){
U32 bytes_written = 0;
U32 blocks_written = 0;
BOOL error=FALSE;
sNktHd nktHead;

#ifdef ENABLE_GEMDOS_IO
S32 fh=-1;
#else
FILE* file=0;
#endif


 if(pOutFileName){
      // create file header
      amTrace("Writing NKT file to: %s\n",pOutFileName);
      Nkt_CreateHeader(&nktHead, pSeq, bCompress);
#ifdef ENABLE_GEMDOS_IO
      fh = Fopen(pOutFileName, FO_WRITE);
      bytes_written+=Fwrite(fh, sizeof(sNktHd),&nktHead);

#else
      file = fopen(pOutFileName, "wb");
      bytes_written+=fwrite(&nktHead, sizeof(sNktHd), 1, file);

#endif


    }

// process tracks
    switch(pSeq->seqType){
    case ST_SINGLE:
        // handle single track sequence
#ifdef ENABLE_GEMDOS_IO
        handleSingleTrack(pSeq, bCompress,fh,&blocks_written,&bytes_written);
#else
        handleSingleTrack(pSeq, bCompress,&file,&blocks_written,&bytes_written);
#endif
        nktHead.NbOfBlocks=blocks_written;
        nktHead.NbOfBytesData=bytes_written;
    break;
    case ST_MULTI:
        // handle multi track sequence
#ifdef ENABLE_GEMDOS_IO
        handleMultiTrack(pSeq, bCompress,fh,&blocks_written,&bytes_written);
#else
        handleMultiTrack(pSeq, bCompress,&file,&blocks_written,&bytes_written);
#endif
        nktHead.NbOfBlocks=blocks_written;
        nktHead.NbOfBytesData=bytes_written;
    break;

    case ST_MULTI_SUB:
    default:
       error=TRUE;
    break;
    };

#ifdef ENABLE_GEMDOS_IO
    if(fh>0){
       Fseek(0, fh, SEEK_SET);
       // update header
       Fwrite(fh, sizeof(sNktHd), &nktHead);

       Fclose(fh); fh=-1;
       amTrace("Stored %d event blocks, %lu kb(%lu bytes) of data.\n",nktHead.NbOfBlocks,nktHead.NbOfBytesData/1024,nktHead.NbOfBytesData);
     }
#else
    if(file){
       fseek(file, 0, SEEK_SET);
       // update header
       fwrite(&nktHead, sizeof(sNktHd), 1, file);

       fclose(file); file=0;
       amTrace("Stored %d event blocks, %lu kb(%lu bytes) of data.\n",nktHead.NbOfBlocks,nktHead.NbOfBytesData/1024,nktHead.NbOfBytesData);
     }
#endif



 if(error!=FALSE) return -1;

 return 0;
}

