/**  Copyright 2007-2020 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#include "nkt.h"
#include "events.h"
#include "timing/miditim.h"

#include "nkt_util.h"
#include "memory.h"

extern uint8 isMidiChannelEvent(const uint8 byteEvent);
extern uint8 isMidiRTorSysex(const uint8 byteEvent);

// helper functions for determining amount of data stored in midi file before actual conversion

void collectNoteOffInfo(uint8 **pMidiData, sRunningStatus_t *rs, sMidiTrackInfo_t* bufferInfo){

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


void collectNoteOnInfo(uint8 **pMidiData, sRunningStatus_t *rs, sMidiTrackInfo_t* bufferInfo){

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

void collectNoteAftInfo(uint8 **pMidiData, sRunningStatus_t *rs, sMidiTrackInfo_t* bufferInfo){

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


void collectControllerEventInfo(uint8 **pMidiData, sRunningStatus_t *rs, sMidiTrackInfo_t* bufferInfo){

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

void collectProgramChangeInfo(uint8 **pMidiData, sRunningStatus_t *rs, sMidiTrackInfo_t* bufferInfo){

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

void collectChannelAftInfo(uint8 **pMidiData, sRunningStatus_t *rs,sMidiTrackInfo_t* bufferInfo){

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

void collectPitchBendInfo(uint8 **pMidiData, sRunningStatus_t *rs,sMidiTrackInfo_t* bufferInfo){

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


void collectMetaEventInfo( uint32 delta, uint8 **pMidiData, sRunningStatus_t *rs, sMidiTrackInfo_t* bufferInfo, bool *bEOT){

uint8 size=0;
uint32 metaLenght=0;

/*get meta event type */
(*pMidiData)++;
uint8 metaType=*(*pMidiData);
(*pMidiData)++;

// get VLQ
metaLenght=readVLQ((*pMidiData),&size);
(*pMidiData)=(*pMidiData)+size;

 switch(metaType){
    case MT_SEQ_NB:{
     amTrace("Meta SEQUENCE NB\n");
    }break;
    case MT_TEXT:{
     amTrace("Meta TEXT\n");
    }break;
    case MT_COPYRIGHT:{
     amTrace("Meta COPYRIGHT\n");
    }break;
    case MT_SEQNAME:{
     amTrace("Meta SEQUENCE NAME\n");
    }break;
    case MT_INSTRNAME:{
     amTrace("Meta INSTRUMENT NAME\n");
    }break;
    case MT_LYRICS:{
     amTrace("Meta LYRICS\n");
    }break;
    case MT_MARKER:{
     amTrace("Meta MARKER\n");
    }break;
    case MT_CUEPOINT:{
     amTrace("Meta CUEPOINT\n");
    }break;
    case MT_PROGRAM_NAME:{
     amTrace("Meta PROGRAM NAME\n");
    }break;
    case MT_DEVICE_NAME:{
        amTrace("Meta DEVICE NAME\n");
    }break;
    case MT_CH_PREFIX:{
        amTrace("Meta CH PREFIX\n");
    }break;
    case MT_MIDI_CH:{
        amTrace("Meta MIDI CH\n");
    }break;
    case MT_MIDI_PORT:{
        amTrace("Meta MIDI PORT\n");
    }break;
    case MT_EOT:{
     amTrace("Meta EOT\n");

     //vlq
     uint32 deltaVLQ=0;
     bufferInfo->eventsBlockSize += WriteVarLen(delta, (uint8 *)&deltaVLQ);;

     // event block
     bufferInfo->eventsBlockSize+=sizeof(sNktBlock_t);

     // no data to write, just count as event block
     ++(bufferInfo->nbOfBlocks);

     *bEOT=TRUE;
    }break;
    case MT_SET_TEMPO:{
     amTrace("Meta Change tempo\n");

     //vlq
     uint32 deltaVLQ=0;
     bufferInfo->eventsBlockSize += WriteVarLen(delta, (uint8 *)&deltaVLQ);;

     // event block
     bufferInfo->eventsBlockSize+=sizeof(sNktBlock_t);

     bufferInfo->bufPos+=(NKT_UMAX*sizeof(uint32))+1; //4 * uint32 + 1*uint32 value
      ++(bufferInfo->nbOfBlocks);
    } break;

    case MT_SMPTE_OFFSET:{amTrace("Meta SMPTE offset\n");}break;
    case MT_TIME_SIG:{amTrace("Meta TIME SIGNATURE\n");}break;
    case MT_KEY_SIG:{amTrace("Meta KEY SIGNATURE\n");}break;
    case MT_SEQ_SPEC:{amTrace("Meta SEQ SPEC\n");}break;
 };

(*pMidiData)+=metaLenght;

}

void collectSysexInfo(uint8 **pMidiData, sRunningStatus_t *rs, sMidiTrackInfo_t* bufferInfo){
uint32 ulCount=0;

    rs->recallRS=0;

    while( (*(*pMidiData))!=EV_EOX){
        (*pMidiData)++;
        /* count Sysex msg data bytes */
        ++ulCount;
    }

    bufferInfo->bufPos+=ulCount;
}



/////////////////////////////////////////   collect midi info
void collectMidiEventInfo(const uint32 delta, uint8 **pCmd, sRunningStatus_t *rs, sMidiTrackInfo_t* bufferInfo , bool *bEOF){
    uint8 usSwitch=0;
    uint8 ubSize=0;

    /* handling of running status */
    /* if byte is not from 0x08-0x0E range then recall last running status AND set recallStatus = 1 */
    /* else set recallStatus = 0 and do nothing special */
    ubSize=*(*pCmd);

     if( (!(isMidiChannelEvent(ubSize))&&(rs->recallRS==1)&&(!(isMidiRTorSysex(ubSize))))){
      /* recall last cmd byte */
      usSwitch = rs->runningStatus&0xF0;
     }else{
      /* check if the new cmd is the system one */
      rs->recallRS=0;

      if((isMidiRTorSysex(ubSize))){
           usSwitch=ubSize;
      }else{
        usSwitch=ubSize;
        usSwitch=usSwitch&0xF0;
       }
      }

      /* decode event and write it to our custom structure */
             switch(usSwitch){
                case EV_NOTE_OFF:
                  amTrace("delta: %lu NOTE OFF\n", delta);
                  collectNoteOffInfo(pCmd,rs, bufferInfo);
                break;
                case EV_NOTE_ON:
                  amTrace("delta: %lu NOTE ON\n", delta);
                  collectNoteOnInfo(pCmd,rs, bufferInfo);
                break;
                case EV_NOTE_AFTERTOUCH:
                  amTrace("delta: %lu NOTE AFT\n", delta);
                  collectNoteAftInfo(pCmd,rs, bufferInfo);
                break;
                case EV_CONTROLLER:
                  amTrace("delta: %lu CONTROLLER\n", delta);
                  collectControllerEventInfo(pCmd,rs, bufferInfo );
                break;
                case EV_PROGRAM_CHANGE:
                  amTrace("delta: %lu PROGRAM CHANGE\n", delta);
                  collectProgramChangeInfo(pCmd,rs, bufferInfo);
                break;
                case EV_CHANNEL_AFTERTOUCH:
                  amTrace("delta: %lu NOTE AFT\n", delta);
                  collectChannelAftInfo(pCmd,rs, bufferInfo);
                break;
                case EV_PITCH_BEND:
                 amTrace("delta: %lu PITCH BEND\n", delta);
                 collectPitchBendInfo(pCmd,rs, bufferInfo);
                break;
                case EV_META:
                 amTrace("delta: %lu ", delta);
                 collectMetaEventInfo(delta, pCmd, rs, bufferInfo, bEOF);
                break;
                case EV_SOX:                                                        /* SySEX midi exclusive */
                  amTrace("delta: %lu SYSEX\n", delta);                             /* cancel out midi running status */
                  collectSysexInfo(pCmd,rs, bufferInfo);
                break;
                case SC_MTCQF:
                  amTrace("delta: %lu SC_MTCQF\n", delta);
                  rs->recallRS=0;                        /* Midi time code quarter frame, 1 byte */
                  amTrace((const uint8*)"Event: System common MIDI time code qt frame\n");
                  ++(*pCmd);
                  ++(*pCmd);
                break;
              case SC_SONG_POS_PTR:
                  amTrace((const uint8*)"Event: System common Song position pointer\n");
                  rs->recallRS=0;                      /* Song position pointer, 2 data bytes */
                   ++(*pCmd);
                   ++(*pCmd);
                   ++(*pCmd);
                break;
                case SC_SONG_SELECT:              /* Song select 0-127, 1 data byte*/
                  amTrace((const uint8*)"Event: System common Song select\n");
                  rs->recallRS=0;
                  ++(*pCmd);
                  ++(*pCmd);
                break;
                case SC_UNDEF1:                   /* undefined */
                case SC_UNDEF2:                   /* undefined */
                  amTrace((const uint8*)"Event: System common not defined.\n");
                  rs->recallRS=0;
                  ++(*pCmd);
                break;
                case SC_TUNE_REQUEST:             /* tune request, no data bytes */
                  amTrace((const uint8*)"Event: System tune request.\n");
                  rs->recallRS=0;
                  ++(*pCmd);
                break;
                default:{
                  amTrace((const uint8*)"Event: Unknown type: %d\n",(*pCmd));
                  /* unknown event, do nothing or maybe throw error? */
                }break;
     } //end switch

}

//
uint32 collectMidiTrackInfo(void *pMidiData, uint16 trackNb, sMidiTrackInfo_t *pBufInfo, bool *bEOT){

    /* process track data, offset the start pointer a little to get directly to track data and decode MIDI events */
    sChunkHeader *pTrackHd=0;
    uint32 trackChunkSize=0;

    void *startTrkPtr=(void *)(((uint8 *)pMidiData)+sizeof(sMThd));
    void *endTrkPtr=0;

    // set first track start
    pTrackHd=(sChunkHeader *)startTrkPtr;

    if(pTrackHd->id!=ID_MTRK){
     amTrace("Error: Cannot find MIDI track [0] chunk. Exiting. \n");
     return 1;
    };

    trackChunkSize=pTrackHd->headLenght;

    // adjust to track start
    startTrkPtr=(void *)( ((uint8 *)pTrackHd) + sizeof(sChunkHeader));
    endTrkPtr=(void *)((uint8*)startTrkPtr + trackChunkSize);
    pTrackHd=endTrkPtr;

    for(int i=0;i<trackNb;++i){

        if(pTrackHd->id!=ID_MTRK){
         amTrace("Error: Cannot find MIDI track [%d] chunk. Exiting. \n", i+1);
         return 1;
        };

        trackChunkSize=pTrackHd->headLenght;

        // adjust to track start
        startTrkPtr=(void *)( ((uint8 *)pTrackHd) + sizeof(sChunkHeader));
        endTrkPtr=(void *)((uint8*)startTrkPtr + trackChunkSize);
        pTrackHd=(sChunkHeader *)endTrkPtr; //next
    }


     // process track events
     uint32 delta=0L;
     uint8 *pCmd=(uint8 *)startTrkPtr;
     uint8 ubSize=0;
     sRunningStatus_t rs;

     (*bEOT)=FALSE;

     // reset running status
     rs.runningStatus=0;
     rs.recallRS=0;

     pBufInfo->nbOfBlocks=0;
     pBufInfo->dataBlockSize=0;
     pBufInfo->eventsBlockSize=0;

     while ( ((pCmd!=endTrkPtr)&&((*bEOT)!=TRUE)) ){
      /* read delta time, pCmd should point to the command data */
      delta=readVLQ(pCmd,&ubSize);
      pCmd+=ubSize;

      collectMidiEventInfo(delta, &pCmd, &rs, pBufInfo ,bEOT);   // todo check error

      uint32 currentDelta = readVLQ(pCmd,&ubSize);

      while((currentDelta==0)&&(pCmd!=endTrkPtr)&&((*bEOT)!=TRUE)){
        pCmd+=ubSize;
        collectMidiEventInfo(0,&pCmd, &rs, pBufInfo,bEOT);
        currentDelta = readVLQ(pCmd,&ubSize);
      }

      if(pBufInfo->bufPos>0){
          uint32 deltaVLQ=0;
          pBufInfo->eventsBlockSize += WriteVarLen(delta, (uint8 *)&deltaVLQ);

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



