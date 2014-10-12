/**  Copyright 2007-2014 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#include "nkt.h"
#include "events.h"
#include "timing/miditim.h"

#include "nkt_util.h"
#include "memory.h"

// helper functions for determining amount of data stored in midi file before actual conversion

U32 collectNoteOffInfo(U8 **pMidiData, sRunningStatus_t *rs, sMidiTrackInfo_t* bufferInfo){

    if(rs->recallRS==0){
        /* save last running status */
        rs->runningStatus=*(*pMidiData);
        bufferInfo->bufPos++;

        /* now we can recall former running status next time */
        rs->recallRS=1;
        (*pMidiData)++;
    }

      bufferInfo->bufPos++;
      bufferInfo->bufPos++;

     (*pMidiData)=(*pMidiData)+sizeof(sNoteOff_t);
}


U32 collectNoteOnInfo(U8 **pMidiData, sRunningStatus_t *rs, sMidiTrackInfo_t* bufferInfo){

if(rs->recallRS==0){
  /* save last running status */
  rs->runningStatus=*(*pMidiData);
   bufferInfo->bufPos++;

  /* now we can recall former running status next time */
  rs->recallRS=1;
  (*pMidiData)++;
 }

  bufferInfo->bufPos++;
  bufferInfo->bufPos++;

 (*pMidiData)=(*pMidiData)+sizeof(sNoteOn_t);
}

U32 collectNoteAftInfo(U8 **pMidiData, sRunningStatus_t *rs, sMidiTrackInfo_t* bufferInfo){

  if(rs->recallRS==0){
   /* save last running status */
   rs->runningStatus=*(*pMidiData);
    bufferInfo->bufPos++;

   /* now we can recall former running status next time */
   rs->recallRS=1;
   (*pMidiData)++;
  }
   bufferInfo->bufPos++;
   bufferInfo->bufPos++;

  (*pMidiData)=(*pMidiData)+sizeof(sNoteAft_t);
}


U32 collectControllerEventInfo(U8 **pMidiData, sRunningStatus_t *rs, sMidiTrackInfo_t* bufferInfo){

if(rs->recallRS==0){
    /* save last running status */
    rs->runningStatus=*(*pMidiData);
    bufferInfo->bufPos++;

    /* now we can recall former running status next time */
    rs->recallRS=1;
    (*pMidiData)++;
 }
  bufferInfo->bufPos++;
  bufferInfo->bufPos++;

 (*pMidiData)=(*pMidiData)+sizeof(sController_t);
}

U32 collectProgramChangeInfo(U8 **pMidiData, sRunningStatus_t *rs, sMidiTrackInfo_t* bufferInfo){

if(rs->recallRS==0){
  /* save last running status */
  rs->runningStatus=*(*pMidiData);
   bufferInfo->bufPos++;

  /* now we can recall former running status next time */
  rs->recallRS=1;
  (*pMidiData)++;
}
  bufferInfo->bufPos++;
 (*pMidiData)=(*pMidiData) + sizeof(sProgramChange_t);
}

U32 collectChannelAftInfo(U8 **pMidiData, sRunningStatus_t *rs,sMidiTrackInfo_t* bufferInfo){

if(rs->recallRS==0){
  /* save last running status */
  rs->runningStatus=*(*pMidiData);
  bufferInfo->bufPos++;

  /* now we can recall former running status next time */
  rs->recallRS=1;
  (*pMidiData)++;
}

 bufferInfo->bufPos++;
 (*pMidiData)=(*pMidiData)+sizeof(sChannelAft_t);
}

U32 collectPitchBendInfo(U8 **pMidiData, sRunningStatus_t *rs,sMidiTrackInfo_t* bufferInfo){

    if(rs->recallRS==0){
        /* save last running status */
        rs->runningStatus=*(*pMidiData);
        bufferInfo->bufPos++;

        /* now we can recall former running status next time */
        rs->recallRS=1;
        (*pMidiData)++;
    }

    bufferInfo->bufPos++;
    bufferInfo->bufPos++;

    (*pMidiData)=(*pMidiData)+sizeof(sPitchBend_t);
}


U32 collectMetaEventInfo( U32 delta, U8 **pMidiData, sRunningStatus_t *rs, sMidiTrackInfo_t* bufferInfo, BOOL *bEOT){

U8 size=0;
U32 metaLenght=0;

/*get meta event type */
(*pMidiData)++;
U8 metaType=*(*pMidiData);
(*pMidiData)++;

// get VLQ
metaLenght=readVLQ((*pMidiData),&size);
(*pMidiData)=(*pMidiData)+size;

 switch(metaType){
    case MT_SEQ_NB:{}break;
    case MT_TEXT:
    case MT_COPYRIGHT:{}break;
    case MT_SEQNAME:{}break;
    case MT_INSTRNAME:{}break;
    case MT_LYRICS:{}break;
    case MT_MARKER:{}break;
    case MT_CUEPOINT:{}break;
    case MT_PROGRAM_NAME:{}break;
    case MT_DEVICE_NAME:{}break;
    case MT_CH_PREFIX:{}break;
    case MT_MIDI_CH:{}break;
    case MT_MIDI_PORT:{}break;
    case MT_EOT:{
     amTrace("Meta EOT\n");

     //vlq
     U32 deltaVLQ=0;
     bufferInfo->eventsBlockSize += WriteVarLen(delta, (U8 *)&deltaVLQ);;

     // event block
     bufferInfo->eventsBlockSize+=sizeof(sNktBlock_t);

     // no data to write, just count as event block
     ++(bufferInfo->nbOfBlocks);

     *bEOT=TRUE;
    }break;
    case MT_SET_TEMPO:{
     amTrace("Meta Change tempo\n");

     //vlq
     U32 deltaVLQ=0;
     bufferInfo->eventsBlockSize += WriteVarLen(delta, (U8 *)&deltaVLQ);;

     // event block
     bufferInfo->eventsBlockSize+=sizeof(sNktBlock_t);

     bufferInfo->bufPos+=(NKT_UMAX*sizeof(U32))+1; //4 * U32 + 1*U32 value
      ++(bufferInfo->nbOfBlocks);
    } break;

    case MT_SMPTE_OFFSET:{}break;
    case MT_TIME_SIG:{}break;
    case MT_KEY_SIG:{}break;
    case MT_SEQ_SPEC:{}break;
 };

(*pMidiData)+=metaLenght;

}

U32 collectSysexInfo(U8 **pMidiData, sRunningStatus_t *rs, sMidiTrackInfo_t* bufferInfo){
U32 ulCount=0;

    rs->recallRS=0;

    while( (*(*pMidiData))!=EV_EOX){
        (*pMidiData)++;
        /* count Sysex msg data bytes */
        ++ulCount;
    }

    bufferInfo->bufPos+=ulCount;
}



/////////////////////////////////////////   collect midi info
U32 collectMidiEventInfo(const U32 delta, U8 **pCmd, sRunningStatus_t *rs, sMidiTrackInfo_t* bufferInfo , BOOL *bEOF){
    U8 usSwitch=0;
    U8 ubSize=0;
    U32 iError=0;

    /* handling of running status */
    /* if byte is not from 0x08-0x0E range then recall last running status AND set recallStatus = 1 */
    /* else set recallStatus = 0 and do nothing special */
    ubSize=*(*pCmd);

     if( (!(isMidiChannelEvent(ubSize))&&(rs->recallRS==1)&&(!(isMidiRTorSysex(ubSize))))){
      /* recall last cmd byte */
      usSwitch = ((rs->runningStatus>>4)&0x0F);
     }else{
      /* check if the new cmd is the system one */
      rs->recallRS=0;

      if((isMidiRTorSysex(ubSize))){
           usSwitch=ubSize;
      }else{
        usSwitch=ubSize;
        usSwitch=((usSwitch>>4)&0x0F);
       }
      }

      /* decode event and write it to our custom structure */
             switch(usSwitch){
                case EV_NOTE_OFF:
                  amTrace("delta: %lu NOTE OFF\n", delta);
                  iError=collectNoteOffInfo(pCmd,rs, bufferInfo);
                break;
                case EV_NOTE_ON:
                  amTrace("delta: %lu NOTE ON\n", delta);
                  iError=collectNoteOnInfo(pCmd,rs, bufferInfo);
                break;
                case EV_NOTE_AFTERTOUCH:
                  amTrace("delta: %lu NOTE AFT\n", delta);
                  iError=collectNoteAftInfo(pCmd,rs, bufferInfo);
                break;
                case EV_CONTROLLER:
                  amTrace("delta: %lu CONTROLLER\n", delta);
                  iError=collectControllerEventInfo(pCmd,rs, bufferInfo );
                break;
                case EV_PROGRAM_CHANGE:
                  amTrace("delta: %lu PROGRAM CHANGE\n", delta);
                  iError=collectProgramChangeInfo(pCmd,rs, bufferInfo);
                break;
                case EV_CHANNEL_AFTERTOUCH:
                   amTrace("delta: %lu NOTE AFT\n", delta);
                  iError=collectChannelAftInfo(pCmd,rs, bufferInfo);
                break;
                case EV_PITCH_BEND:
                 amTrace("delta: %lu PITCH BEND\n", delta);
                  iError=collectPitchBendInfo(pCmd,rs, bufferInfo);
                break;
                case EV_META:
                 amTrace("delta: %lu META\n", delta);
                 iError=collectMetaEventInfo(delta, pCmd, rs, bufferInfo, bEOF);

                break;
                case EV_SOX:                          	/* SySEX midi exclusive */
                  amTrace("delta: %lu SYSEX\n", delta);
                                    /* cancel out midi running status */
                  iError=(S16)collectSysexInfo(pCmd,rs, bufferInfo);
                break;
                case SC_MTCQF:
                  amTrace("delta: %lu SC_MTCQF\n", delta);
                  rs->recallRS=0;                        /* Midi time code quarter frame, 1 byte */
                  amTrace((const U8*)"Event: System common MIDI time code qt frame\n");
                  (*pCmd)++;
                  (*pCmd)++;
                break;
              case SC_SONG_POS_PTR:
                  amTrace((const U8*)"Event: System common Song position pointer\n");
                  rs->recallRS=0;                      /* Song position pointer, 2 data bytes */
                   (*pCmd)++;
                   (*pCmd)++;
                   (*pCmd)++;
                break;
                case SC_SONG_SELECT:              /* Song select 0-127, 1 data byte*/
                  amTrace((const U8*)"Event: System common Song select\n");
                  rs->recallRS=0;
                  (*pCmd)++;
                  (*pCmd)++;
                break;
                case SC_UNDEF1:                   /* undefined */
                case SC_UNDEF2:                   /* undefined */
                  amTrace((const U8*)"Event: System common not defined.\n");
                  rs->recallRS=0;
                  (*pCmd)++;
                break;
                case SC_TUNE_REQUEST:             /* tune request, no data bytes */
                  amTrace((const U8*)"Event: System tune request.\n");
                  rs->recallRS=0;
                 (*pCmd)++;
                break;
                default:{
                  amTrace((const U8*)"Event: Unknown type: %d\n",(*pCmd));
                  /* unknown event, do nothing or maybe throw error? */
                }break;
     } //end switch

    return iError;
}

//
U32 collectMidiTrackInfo(void *pMidiData, sMidiTrackInfo_t *pBufInfo, BOOL *bEOT){

    /* process track data, offset the start pointer a little to get directly to track data and decode MIDI events */
    sChunkHeader *pTrackHd=0;
    U32 trackChunkSize=0;

    void *startTrkPtr=(void *)(((U8 *)pMidiData)+sizeof(sMThd));
    void *endTrkPtr=0;

    pTrackHd=(sChunkHeader *)startTrkPtr;

    if(pTrackHd->id!=ID_MTRK){
     amTrace("Error: Cannot find MIDI track chunk. Exiting. \n");
     return 1;
    };

    trackChunkSize=pTrackHd->headLenght;

    // adjust to track start
    startTrkPtr=(void *)( ((U8 *)pTrackHd) + sizeof(sChunkHeader));
    endTrkPtr=(void *)((U8*)pTrackHd + trackChunkSize);

     // process track events
     U32 delta=0L;
     S32 iError=0;
     U8 *pCmd=(U8 *)startTrkPtr;
     U8 ubSize=0;
     sRunningStatus_t rs;

     (*bEOT)=FALSE;

     // reset running status
     rs.runningStatus=0;
     rs.recallRS=0;

     pBufInfo->nbOfBlocks=0;
     pBufInfo->dataBlockSize=0;
     pBufInfo->eventsBlockSize=0;

     while ( ((pCmd!=endTrkPtr)&&((*bEOT)!=TRUE)&&(iError>=0)) ){
      /* read delta time, pCmd should point to the command data */
      delta=readVLQ(pCmd,&ubSize);
      pCmd+=ubSize;

      iError=collectMidiEventInfo(delta, &pCmd, &rs, pBufInfo ,bEOT);   // todo check error

      U32 currentDelta = readVLQ(pCmd,&ubSize);

      while((currentDelta==0)&&(pCmd!=endTrkPtr)&&((*bEOT)!=TRUE)&&(iError>=0)){
        pCmd+=ubSize;
        iError=collectMidiEventInfo(0,&pCmd, &rs, pBufInfo,bEOT);
        currentDelta = readVLQ(pCmd,&ubSize);
      }

      if(pBufInfo->bufPos>0){
          U32 deltaVLQ=0;
          pBufInfo->eventsBlockSize += WriteVarLen(delta, (U8 *)&deltaVLQ);

          // event block
          pBufInfo->eventsBlockSize+=sizeof(sNktBlock_t);
          pBufInfo->dataBlockSize+=pBufInfo->bufPos;

          pBufInfo->bufPos=0;
          ++(pBufInfo->nbOfBlocks);
      }


     } /*end of decode events loop */

     // OK
     return 0;
 }



