
/**  Copyright 2007-2020 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#include "nkt.h"
#include "nkt_util.h"

#include "events.h"
#include "timing/miditim.h"
#include "memory/linalloc.h"
#include "midi.h"
#include "lzo/minilzo.h"

#ifdef ENABLE_GEMDOS_IO
#include <mint/ostruct.h>
#include <mint/osbind.h>
#include "fmio.h"
#endif

#include <stdio.h>

extern uint32 collectMidiTrackInfo(void *pMidiData, uint16 trackNb, sMidiTrackInfo_t *pBufInfo, bool *bEOT);

// from mparser.c
uint8  isMidiChannelEvent(const uint8 byteEvent){
    if(( ((byteEvent&0xF0)>=0x80) && ((byteEvent&0xF0)<=0xE0))) {
        return 1;
    }
    
    return 0;
}

uint8 isMidiRTorSysex(const uint8 byteEvent){

    if( ((byteEvent>=(uint8)0xF0)&&(byteEvent<=(uint8)0xFF)) ){
      /* it is! */
        return (1);
    }
 
  return (0);
}

/* combine bytes function for pitch bend */
uint16 amCombinePitchBendBytes(const uint8 bFirst, const uint8 bSecond){
    uint16 val;
    val = (uint16)bSecond;
    val<<=7;
    val|=(uint16)bFirst;
 return(val);
}

//
#define OUT_BUFFER_SIZE 1024  // should be sufficient if there are no big SysEx messages
                              // TODO: make it configurable (?)
typedef struct sBufferInfo{
 uint8 buffer[OUT_BUFFER_SIZE];  // temp buffer for gathering data that go to data buffer
 uint32 bufPos;                  // buffer[] writing position

 uint32 eventsBlockOffset; //where we are writing in events block buffer
 uint32 dataBlockOffset;   // where we are writing in data block buffer

} sBufferInfo_t;


void processNoteOff(uint8 **pMidiData, sRunningStatus_t *rs, sBufferInfo_t* bufferInfo){
    sNoteOff_t *pNoteOff=0;

    if(rs->recallRS==0){
        /* save last running status */
        rs->runningStatus=*(*pMidiData);

        bufferInfo->buffer[bufferInfo->bufPos++]=*(*pMidiData);

        /* now we can recall former running status next time */
        rs->recallRS=1;

        ++(*pMidiData);
        pNoteOff=(sNoteOff_t *)(*pMidiData);

    }else{

        /* recall last cmd status */
        /* and get parameters as usual */

        pNoteOff=(sNoteOff_t *)(*pMidiData);
    }

    bufferInfo->buffer[bufferInfo->bufPos++]=pNoteOff->noteNb;
    bufferInfo->buffer[bufferInfo->bufPos++]=pNoteOff->velocity;

    amTrace(" n: %d  v: %d\n",pNoteOff->noteNb, pNoteOff->velocity);
    (*pMidiData)=(*pMidiData)+sizeof(sNoteOff_t);

}


void processNoteOn(uint8 **pMidiData, sRunningStatus_t *rs, sBufferInfo_t* bufferInfo){
    sNoteOn_t *pNoteOn=0;

    if(rs->recallRS==0){
    /* save last running status */
        rs->runningStatus=*(*pMidiData);
        bufferInfo->buffer[bufferInfo->bufPos++]=*(*pMidiData);

    /* now we can recall former running status next time */
        rs->recallRS=1;
        ++(*pMidiData);

        // get channel from running status
        pNoteOn=(sNoteOn_t *)(*pMidiData);
    }else{
        pNoteOn=(sNoteOn_t *)(*pMidiData);
    }

    bufferInfo->buffer[bufferInfo->bufPos++]=pNoteOn->noteNb;
    bufferInfo->buffer[bufferInfo->bufPos++]=pNoteOn->velocity;

    amTrace(" n: %d  v: %d\n",pNoteOn->noteNb, pNoteOn->velocity);
    (*pMidiData)=(*pMidiData)+sizeof(sNoteOn_t);

}

void processNoteAft(uint8 **pMidiData, sRunningStatus_t *rs, sBufferInfo_t* bufferInfo){
  sNoteAft_t *pNoteAft=0;

  if(rs->recallRS==0){
   /* save last running status */
   rs->runningStatus=*(*pMidiData);
   bufferInfo->buffer[bufferInfo->bufPos++]=*(*pMidiData);

   /* now we can recall former running status next time */
   rs->recallRS=1;
   (*pMidiData)++;
    pNoteAft=(sNoteAft_t *)(*pMidiData);
  }else{
    pNoteAft=(sNoteAft_t *)(*pMidiData);
  }

  bufferInfo->buffer[bufferInfo->bufPos++]=pNoteAft->noteNb;
  bufferInfo->buffer[bufferInfo->bufPos++]=pNoteAft->pressure;

  amTrace(" n: %d  p: %d\n",pNoteAft->noteNb, pNoteAft->pressure);
  (*pMidiData)=(*pMidiData)+sizeof(sNoteAft_t);
}


void processControllerEvent(uint8 **pMidiData, sRunningStatus_t *rs, sBufferInfo_t* bufferInfo){
    sController_t *pContrEv=0;

    if(rs->recallRS==0){
        /* save last running status */
        rs->runningStatus=*(*pMidiData);
        bufferInfo->buffer[bufferInfo->bufPos++]=*(*pMidiData);

        /* now we can recall former running status next time */
        rs->recallRS=1;
        ++(*pMidiData);
        pContrEv=(sController_t *)(*pMidiData);
    }else{
        pContrEv=(sController_t *)(*pMidiData);
    }

    bufferInfo->buffer[bufferInfo->bufPos++]=pContrEv->controllerNb;
    bufferInfo->buffer[bufferInfo->bufPos++]=pContrEv->value;

    amTrace(" c: %d  v: %d\n",pContrEv->controllerNb, pContrEv->value);
    (*pMidiData)=(*pMidiData)+sizeof(sController_t);
}

void processProgramChange(uint8 **pMidiData, sRunningStatus_t *rs, sBufferInfo_t* bufferInfo){
    sProgramChange_t *pPC=0;

    if(rs->recallRS==0){
        /* save last running status */
        rs->runningStatus=*(*pMidiData);
        bufferInfo->buffer[bufferInfo->bufPos++]=*(*pMidiData);

        /* now we can recall former running status next time */
        rs->recallRS=1;
        ++(*pMidiData);
        pPC=(sProgramChange_t *)(*pMidiData);
    }else{
        pPC=(sProgramChange_t *)(*pMidiData);
    }

    amTrace(" p: %d \n",pPC->programNb);
    bufferInfo->buffer[bufferInfo->bufPos++]=pPC->programNb;

    (*pMidiData)=(*pMidiData) + sizeof(sProgramChange_t);
}

void processChannelAft(uint8 **pMidiData, sRunningStatus_t *rs,sBufferInfo_t* bufferInfo){
sChannelAft_t *pChAft=0;

    if(rs->recallRS==0){
        /* save last running status */
        rs->runningStatus=*(*pMidiData);
        bufferInfo->buffer[bufferInfo->bufPos++]=*(*pMidiData);

        /* now we can recall former running status next time */
        rs->recallRS=1;
        ++(*pMidiData);
        pChAft=(sChannelAft_t *)(*pMidiData);
    }else{
        pChAft=(sChannelAft_t *)(*pMidiData);
    }

    amTrace(" press: %d \n",pChAft->pressure);

    bufferInfo->buffer[bufferInfo->bufPos++]=pChAft->pressure;

    (*pMidiData)=(*pMidiData)+sizeof(sChannelAft_t);
}

void processPitchBend(uint8 **pMidiData, sRunningStatus_t *rs,sBufferInfo_t* bufferInfo){
    sPitchBend_t *pPitchBend=0;

    if(rs->recallRS==0){
        /* save last running status */
        rs->runningStatus=*(*pMidiData);
        bufferInfo->buffer[bufferInfo->bufPos++]=*(*pMidiData);

        /* now we can recall former running status next time */
        rs->recallRS=1;
        ++(*pMidiData);
        pPitchBend=(sPitchBend_t *)(*pMidiData);
     }else{
        pPitchBend=(sPitchBend_t *)(*pMidiData);
    }

    bufferInfo->buffer[bufferInfo->bufPos++]=pPitchBend->LSB;
    bufferInfo->buffer[bufferInfo->bufPos++]=pPitchBend->MSB;

    amTrace(" LSB: %d MSB: %d\n",pPitchBend->LSB,pPitchBend->MSB);

    (*pMidiData)=(*pMidiData)+sizeof(sPitchBend_t);
}

void processMetaEvent( uint32 delta, uint8 **pMidiData, sNktSeq *pSeq, uint16 trackIdx, sRunningStatus_t *rs, sBufferInfo_t* bufferInfo, bool *bEOT){

uint8 size=0;
uint32 metaLenght=0;
sNktBlock_t stBlock;

sNktTrack *pTrk=&pSeq->pTracks[trackIdx];

/*get meta event type */
++(*pMidiData);
uint8 metaType=*(*pMidiData);

++(*pMidiData);

// get VLQ
metaLenght=readVLQ((*pMidiData),&size);
(*pMidiData)=(*pMidiData)+size;

 switch(metaType){

    case MT_EOT:{
     amTrace("Write event block:\t");
     amTrace("delta: %lu Meta End of Track\n", delta);

        stBlock.msgType=NKT_END;
        stBlock.blockSize=0;
        stBlock.bufferOffset=0; //we don't mind

         // write VLQ
         uint32 eventsBufPos=((uint32)pTrk->eventBlocksPtr)+bufferInfo->eventsBlockOffset;
         int32 count=WriteVarLen((int32)delta,(uint8 *)eventsBufPos);
         bufferInfo->eventsBlockOffset+=count;

         // write event info block
         eventsBufPos=((uint32)pTrk->eventBlocksPtr)+bufferInfo->eventsBlockOffset;
         amMemCpy((void *)eventsBufPos,&stBlock,sizeof(sNktBlock_t));
         bufferInfo->eventsBlockOffset+=sizeof(sNktBlock_t);

         //no data to write
        *bEOT=TRUE;
    } break;

    case MT_SET_TEMPO:{
            amTrace("\nWrite event block:\t");
            amTrace("Meta Set Tempo\n");

            stBlock.msgType = NKT_TEMPO_CHANGE;
            stBlock.blockSize = 5 * sizeof(uint32);   //uint32 tempo value + 4*uint32 values (25,50,100,200hz timesteps)
            stBlock.bufferOffset=bufferInfo->dataBlockOffset;

            uint8 ulVal[3] = {0};   /* for retrieving set tempo info */
            uint32 val1=0, val2=0, val3=0;
            amMemCpy(ulVal, (*pMidiData),metaLenght*sizeof(uint8) );

            val1 = ulVal[0], val2 = ulVal[1],val3 = ulVal[2];
            val1=(val1<<16)&0x00FF0000L;
            val2=(val2<<8)&0x0000FF00L;
            val3=(val3)&0x000000FFL;

            // range: 0-8355711 ms, 24 bit value
            val1=val1|val2|val3;
            amTrace("%lu ms per quarter-note\n", val1);

            // write VLQ delta
            uint32 eventsBufPos=((uint32)pTrk->eventBlocksPtr)+bufferInfo->eventsBlockOffset;
            int32 count=WriteVarLen((int32)delta,(uint8 *)eventsBufPos);
            bufferInfo->eventsBlockOffset+=count;

            // write event info block
            eventsBufPos=((uint32)pTrk->eventBlocksPtr)+bufferInfo->eventsBlockOffset;
            amMemCpy((void *)eventsBufPos,&stBlock,sizeof(sNktBlock_t));
            bufferInfo->eventsBlockOffset+=sizeof(sNktBlock_t);

            // write tempo value to data buffer
            eventsBufPos=((uint32)pTrk->eventDataPtr)+bufferInfo->dataBlockOffset;
            amMemCpy((void *)eventsBufPos,&val1,sizeof(uint32));
            bufferInfo->dataBlockOffset+=sizeof(uint32);

            uint32 precalc[NKT_UMAX]={0L};
            uint32 td = pSeq->timeDivision;
            uint32 bpm = 60000000UL / val1;
            uint32 tempPPU = bpm * td;

            amTrace("Precalculating update step for TD: %d, BPM:%d\n",td,bpm);
           // precalculate valuies for different update steps

            for(int i=0;i<NKT_UMAX;++i){
                switch(i){
                    case NKT_U25HZ:{
                        if(tempPPU<0x10000){
                            precalc[i]=((tempPPU*0x10000)/60)/25;
                        }else{
                            precalc[i]=((tempPPU/60)*0x10000)/25;
                        }
                        amTrace("Update step for 25hz: %ld\n",precalc[i]);
                    } break;
                    case NKT_U50HZ:{
                        if(tempPPU<0x10000){
                            precalc[i]=((tempPPU*0x10000)/60)/50;
                        }else{
                            precalc[i]=((tempPPU/60)*0x10000)/50;
                        }
                         amTrace("Update step for 50hz: %ld\n",precalc[i]);
                    } break;
                    case NKT_U100HZ:{
                        if(tempPPU<0x10000){
                            precalc[i]=((tempPPU*0x10000)/60)/100;
                        }else{
                            precalc[i]=((tempPPU/60)*0x10000)/100;
                        }
                         amTrace("Update step for 100hz: %ld\n",precalc[i]);
                    } break;
                    case NKT_U200HZ:{
                        if(tempPPU<0x10000){
                            precalc[i]=((tempPPU*0x10000)/60)/200;
                        }else{
                            precalc[i]=((tempPPU/60)*0x10000)/200;
                        }
                         amTrace("Update step for 200hz: %ld\n",precalc[i]);
                    } break;
                    default:{
                        ASSERT(0);
                        amTrace((const uint8*)"[Error] Invalid timer update value %d\n", i);
                    } break;
                };

           } //end for

           // write precalculated values to data buffer

           eventsBufPos=((uint32)pTrk->eventDataPtr)+bufferInfo->dataBlockOffset;
           amMemCpy((void *)eventsBufPos,(void *)precalc,NKT_UMAX*sizeof(uint32));
           bufferInfo->dataBlockOffset+= NKT_UMAX*sizeof(uint32);

    }break;

    case MT_SEQ_NB:{amTrace("META: MT_SEQ_NB\n");} break;
    case MT_TEXT:{amTrace("META: MT_TEXT \n");} break;
    case MT_COPYRIGHT:{amTrace("META: MT_COPYRIGHT \n");} break;
    case MT_SEQNAME:{amTrace("META: MT_SEQNAME \n");} break;
    case MT_INSTRNAME:{amTrace("META: MT_INSTRNAME \n");} break;
    case MT_LYRICS:{amTrace("META: MT_LYRICS\n");} break;
    case MT_MARKER:{amTrace("META: MT_MARKER \n");} break;
    case MT_CUEPOINT:{amTrace("META: MT_CUEPOINT \n");} break;
    case MT_PROGRAM_NAME:{amTrace("META: MT_PROGRAM_NAME \n");} break;
    case MT_DEVICE_NAME:{amTrace("META: MT_DEVICE_NAME \n");} break;
    case MT_CH_PREFIX:{amTrace("META: MT_CH_PREFIX \n");} break;
    case MT_MIDI_CH:{amTrace("META: MT_MIDI_CH \n");} break;
    case MT_MIDI_PORT:{amTrace("META: MT_MIDI_PORT \n");} break;
    case MT_SMPTE_OFFSET:{amTrace("META: MT_SMPTE_OFFSET \n");}break;
    case MT_TIME_SIG:{amTrace("META: MT_TIME_SIG \n");}break;
    case MT_KEY_SIG:{amTrace("META: MT_KEY_SIG \n");}break;
    case MT_SEQ_SPEC:{amTrace("META: MT_SEQ_SPEC \n");}break;
 };

(*pMidiData)+=metaLenght;

}

void processSysex(uint8 **pMidiData, sRunningStatus_t *rs, sBufferInfo_t* bufferInfo){
uint32 ulCount=0L;
uint8 *pDataPtr=0;

pDataPtr=(*pMidiData); //save SysEX start

while( (*(*pMidiData))!=EV_EOX){
    (*pMidiData)++;
    /* count Sysex msg data bytes */
    ++ulCount;
}

 // copy data
 amMemCpy(&bufferInfo->buffer[bufferInfo->bufPos],pDataPtr,ulCount);
 bufferInfo->bufPos+=ulCount;
}


void processMidiEvent(const uint32 delta, uint8 **pCmd, sRunningStatus_t *rs, sBufferInfo_t* bufferInfo ,sNktSeq *pSeq, uint16 trackNbToProcess, bool *bEOT){
 uint8 usSwitch=0;
 uint8 ubSize=0;
 uint32 iError=0;

 /* handling of running status */
 /* if byte is not from 0x08-0x0E range then recall last running status AND set recallStatus = 1 */
 /* else set recallStatus = 0 and do nothing special */
 ubSize=*(*pCmd);

  if( (!(isMidiChannelEvent(ubSize))&&(rs->recallRS==1)&&(!(isMidiRTorSysex(ubSize))))){
   /* recall last cmd byte */
   usSwitch = ((rs->runningStatus)&0xF0);
  }else{
   /* check if the new cmd is the system one */
   rs->recallRS=0;

   if((isMidiRTorSysex(ubSize))){
        usSwitch=ubSize;
   }else{
     usSwitch=ubSize;
     usSwitch=((usSwitch)&0xF0);
    }
   }

   /* decode event and write it to our custom structure */
          switch(usSwitch){
             case EV_NOTE_OFF:
               amTrace("delta: %lu NOTE OFF\t", delta);
               processNoteOff(pCmd,rs, bufferInfo);
             break;
             case EV_NOTE_ON:
               amTrace("delta: %lu NOTE ON\t", delta);
               processNoteOn(pCmd,rs, bufferInfo);
             break;
             case EV_NOTE_AFTERTOUCH:
               amTrace("delta: %lu NOTE AFT\t", delta);
               processNoteAft(pCmd,rs, bufferInfo);
             break;
             case EV_CONTROLLER:
               amTrace("delta: %lu CONTROLLER\t", delta);
               processControllerEvent(pCmd,rs, bufferInfo );
             break;
             case EV_PROGRAM_CHANGE:
               amTrace("delta: %lu PROGRAM CHANGE\t", delta);
               processProgramChange(pCmd,rs, bufferInfo);
             break;
             case EV_CHANNEL_AFTERTOUCH:
               amTrace("delta: %lu NOTE AFT\t", delta);
               processChannelAft(pCmd,rs, bufferInfo);
             break;
             case EV_PITCH_BEND:
               amTrace("delta: %lu PITCH BEND\t", delta);
               processPitchBend(pCmd,rs, bufferInfo);
             break;
             case EV_META:
              amTrace("delta: %lu META\t", delta);
              processMetaEvent(delta, pCmd, pSeq, trackNbToProcess, rs, bufferInfo, bEOT);
             break;
             case EV_SOX:                          	/* SySEX midi exclusive */
               amTrace("delta: %lu SYSEX\n", delta);
               rs->recallRS=0;                      /* cancel out midi running status */
               processSysex(pCmd,rs, bufferInfo);
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


uint32 midiTrackDataToNkt(void *pMidiData, sNktSeq *pSeq, uint16 trackNbToProcess){

    /* process track data, offset the start pointer a little to get directly to track data and decode MIDI events */
    sChunkHeader *pTrackHd=0;
    uint32 trackChunkSize=0;

    void *startTrkPtr=(void *)(((uint8 *)pMidiData)+sizeof(sMThd));
    void *endTrkPtr=0;

    pTrackHd=(sChunkHeader *)startTrkPtr;

    // adjust to track start
    // set track 0
    if(pTrackHd->id!=ID_MTRK){
        printf( "Error: Cannot find MIDI track [0] chunk. Exiting. \n");
        amTrace("Error: Cannot find MIDI track [0] chunk. Exiting. \n");
        return 1;
    };
    trackChunkSize = pTrackHd->headLenght;

    // adjust to track start
    startTrkPtr=(void *)( ((uint8 *)pTrackHd) + sizeof(sChunkHeader));
    endTrkPtr=(void *)((uint8*)startTrkPtr + trackChunkSize);
    pTrackHd=(sChunkHeader *)endTrkPtr;

    for(int i=0;i<trackNbToProcess;++i){

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
 bool bEOT=FALSE;
 uint8 *pCmd=(uint8 *)startTrkPtr;
 uint8 ubSize=0;
 sNktBlock_t stBlock;
 sRunningStatus_t rs;
 sBufferInfo_t tempBufInfo;

 rs.runningStatus=0;
 rs.recallRS=0;

// clear
 amMemSet(&tempBufInfo,0,sizeof(sBufferInfo_t));

 while ( ((pCmd!=endTrkPtr)&&(bEOT!=TRUE)) ){

  /* read delta time, pCmd should point to the command data */
  delta=readVLQ(pCmd,&ubSize);
  pCmd+=ubSize;

  processMidiEvent(delta, &pCmd, &rs, &tempBufInfo ,pSeq, trackNbToProcess, &bEOT);

  uint32 currentDelta = readVLQ(pCmd,&ubSize);

  while((currentDelta==0)&&(pCmd!=endTrkPtr)&&(bEOT!=TRUE)){
    pCmd+=ubSize;

    processMidiEvent(0,&pCmd, &rs, &tempBufInfo, pSeq, trackNbToProcess,&bEOT);

    currentDelta = readVLQ(pCmd,&ubSize);
  }

  // dump midi event block to contigous events block
  if(tempBufInfo.bufPos>0){

      stBlock.msgType = NKT_MIDIDATA;
      stBlock.blockSize = tempBufInfo.bufPos;
      stBlock.bufferOffset=tempBufInfo.dataBlockOffset;

#ifdef DEBUG_BUILD
      amTrace("[DATA] ");

      for(int j=0; j<stBlock.blockSize; j++){
       amTrace("[%x]",tempBufInfo.buffer[j]);
      }

      amTrace(" [/DATA]\n");
#endif

      // write evnt block and data to buffers
      amTrace("[Write event block]: d: [%lu] type: [%d] size: [%d] startOffset: [%lu]\n",delta,stBlock.msgType,tempBufInfo.bufPos,stBlock.bufferOffset);

      // write VLQ
      uint32 eventsBufPos=((uint32)pSeq->pTracks[trackNbToProcess].eventBlocksPtr)+tempBufInfo.eventsBlockOffset;
      int32 count=WriteVarLen((int32)delta,(uint8 *)eventsBufPos);
      amTrace("[E] delta vlq at [%lu]\n",tempBufInfo.eventsBlockOffset);
      tempBufInfo.eventsBlockOffset+=count;

      // write event info block
      eventsBufPos=((uint32)pSeq->pTracks[trackNbToProcess].eventBlocksPtr)+tempBufInfo.eventsBlockOffset;
      amMemCpy((void *)eventsBufPos,&stBlock,sizeof(sNktBlock_t));
      amTrace("[E] event info at [%lu]\n",tempBufInfo.eventsBlockOffset);
      tempBufInfo.eventsBlockOffset+=sizeof(sNktBlock_t);

      // write to data buffer

      eventsBufPos=((uint32)pSeq->pTracks[trackNbToProcess].eventDataPtr)+tempBufInfo.dataBlockOffset;
      amMemCpy((void *)eventsBufPos, &tempBufInfo.buffer[0], stBlock.blockSize);
      amTrace("[D] event data at [%lu]\n",tempBufInfo.dataBlockOffset);

      tempBufInfo.dataBlockOffset+=stBlock.blockSize;

      // next event block
      amTrace("delta: [%lu] type:[%d] block size:[%u] bytes \n",delta, stBlock.msgType, stBlock.blockSize);

      // clear temp buffer
      tempBufInfo.bufPos=0;
      amMemSet(&(tempBufInfo.buffer[0]), 0L, OUT_BUFFER_SIZE);
  }

 } /*end of decode events loop */

 if(bEOT==FALSE){
        amTrace("EOT meta event not found, appending NKT_END event!\n");
        amTrace("delta: %lu Meta End of Track\n", 0);

        stBlock.msgType=NKT_END;
        stBlock.blockSize=0;
        stBlock.bufferOffset=0; //we don't mind

        // write VLQ
        uint32 eventsBufPos=((uint32)pSeq->pTracks[trackNbToProcess].eventBlocksPtr)+tempBufInfo.eventsBlockOffset;
        int32 count=WriteVarLen((int32)0,(uint8 *)pSeq->pTracks[trackNbToProcess].eventBlocksPtr);
        tempBufInfo.eventsBlockOffset+=count;

        // write event info block
        eventsBufPos=((uint32)pSeq->pTracks[trackNbToProcess].eventBlocksPtr)+tempBufInfo.eventsBlockOffset;
        amMemCpy((void *)eventsBufPos,&stBlock,sizeof(sNktBlock_t));
        tempBufInfo.eventsBlockOffset+=sizeof(sNktBlock_t);
 }

    // OK
 return 0;
}

sNktSeq *Midi2Nkt(void *pMidiData, const uint8 *pOutFileName, const bool bCompress){

sBufferInfo_t BufferInfo;
sNktSeq *pNewSeq=0;
bool bEOT=FALSE;
uint16 nbOfTracks=((sMThd *)pMidiData)->nTracks;

sMidiTrackInfo_t *arMidiInfo = (sMidiTrackInfo_t *)amMallocEx(sizeof(sMidiTrackInfo_t)*nbOfTracks,PREFER_TT);

if(arMidiInfo==0){
    amTrace("[MIDI2NKT] No memory for track info. Exiting...\n");
    printf("[MIDI2NKT] No memory for track info. Exiting \n");
    return 0;
}

amMemSet(&BufferInfo, 0L, sizeof(sBufferInfo_t));
amMemSet(arMidiInfo, 0L, sizeof(sMidiTrackInfo_t)*nbOfTracks);

//collect track info from each track
for(uint16 i=0;i<nbOfTracks;++i){

    if(collectMidiTrackInfo(pMidiData,i,&arMidiInfo[i],&bEOT)!=0){
        amTrace("[MIDI2NKT]  MIDI track [%d] parse error. Exiting...\n",i);
        printf("[MIDI2NKT]  MIDI track [%d] parse error. Exiting \n",i);
        return 0;
    }

    // special case if midi track data isn't properly terminated with EOT meta event
    // like mus files or some mid files

    if(bEOT==FALSE){
        amTrace("No EOT in midi data found, adding EOT meta event...\n");
        arMidiInfo[i].eventsBlockSize += 1;
        arMidiInfo[i].eventsBlockSize+=sizeof(sNktBlock_t);
        ++arMidiInfo[i].nbOfBlocks;
    }
    amTrace("[Midi track #%d]\nEvents:[%ld],\nEvent block: [%ld] bytes,\nData block: [%ld] bytes\n",i,arMidiInfo[i].nbOfBlocks,arMidiInfo[i].eventsBlockSize,arMidiInfo[i].dataBlockSize);

}


// now MidiInfo holds information about number of event blocks
// and amount of bytes wee need to store midi data

// reserve memory for sequence header
pNewSeq = (sNktSeq *)amMallocEx(sizeof(sNktSeq),PREFER_TT);

if(pNewSeq==0) {
    amTrace("[MIDI2NKT] Fatal error, couldn't allocate memory for header.\n");
    amFree(arMidiInfo);
    return 0;
}

// clear memory
amMemSet(pNewSeq,0,sizeof(sNktSeq));

// init header
pNewSeq->currentUpdateFreq=NKT_U200HZ;
pNewSeq->sequenceState |= NKT_PLAY_ONCE;
pNewSeq->defaultTempo.tempo=DEFAULT_MPQN;
pNewSeq->currentTempo.tempo=DEFAULT_MPQN;
pNewSeq->timeDivision=DEFAULT_PPQN;
pNewSeq->version=NKT_VERSION;

pNewSeq->nbOfTracks=nbOfTracks;  // save nb of tracks
pNewSeq->pTracks=(sNktTrack *) amMallocEx(nbOfTracks*sizeof(sNktTrack), PREFER_TT);

if(pNewSeq->pTracks==NULL){
     amFree(pNewSeq);
     amTrace("[MIDI2NKT] Fatal error, couldn't reserve memory for track data.\n");
     return 0;
}

// reserve memory for all tracks
for(int i=0;i<nbOfTracks;++i){
    sNktTrack *pTrk=&pNewSeq->pTracks[i];

    pTrk->nbOfBlocks = arMidiInfo[i].nbOfBlocks;
    pTrk->eventsBlockBufferSize = arMidiInfo[i].eventsBlockSize;
    pTrk->dataBufferSize = arMidiInfo[i].dataBlockSize;

    // reserve and initialise linear memory buffers
    // for event blocks and data
    if(createLinearBuffer(&(pTrk->lbEventsBuffer),pTrk->eventsBlockBufferSize+255, PREFER_TT)<0){
        amTrace("[MIDI2NKT] Fatal error, couldn't reserve memory for events block buffer.\n");
        amFree(arMidiInfo);
        amFree(pNewSeq);
        return 0;
    }

    if(createLinearBuffer(&(pTrk->lbDataBuffer),pTrk->dataBufferSize+255, PREFER_TT)<0){
      amTrace("[MIDI2NKT] Fatal error, couldn't reserve memory for data buffer block.\n");
      destroyLinearBuffer(&(pNewSeq->pTracks[i].lbEventsBuffer));
      amFree(arMidiInfo);
      amFree(pNewSeq);

      return 0;
    }

    // allocate memory
    amTrace("[MIDI2NKT] Reserve memory track: [%d]:\nNb of events: %lu\nEvents block: %ld\nData block: %ld bytes\n",i, pTrk->nbOfBlocks, pTrk->eventsBlockBufferSize, pTrk->dataBufferSize);

    // allocate contigous / linear memory for pNewSeq->NbOfBlocks events
    uint32 lbAllocAdr=0;
    lbAllocAdr=(uint32) linearBufferAlloc(&(pTrk->lbEventsBuffer), pTrk->eventsBlockBufferSize+255);
    lbAllocAdr+=255;
    lbAllocAdr&=0xfffffff0;
    pTrk->eventBlocksPtr = (uint8 *)lbAllocAdr;

    // alloc memory for data buffer from linear allocator
    lbAllocAdr=(uint32)linearBufferAlloc(&(pTrk->lbDataBuffer), pTrk->dataBufferSize+255);
    lbAllocAdr+=255;
    lbAllocAdr&=0xfffffff0;
    pTrk->eventDataPtr = (uint8*)lbAllocAdr;

    if(pTrk->eventBlocksPtr==0||pTrk->eventDataPtr==0){
        amTrace("[MIDI2NKT] Fatal error, couldn't allocate memory for events block / events data.\n");

        destroyLinearBuffer(&(pTrk->lbDataBuffer));
        destroyLinearBuffer(&(pTrk->lbEventsBuffer));

        amFree(arMidiInfo);
        amFree(pNewSeq);

        return 0;
    }

    // clear memory
        amMemSet((void *)pTrk->eventBlocksPtr,0, pTrk->eventsBlockBufferSize);
        amMemSet((void *)pTrk->eventDataPtr,0, pTrk->dataBufferSize);

} //end reserve memory


// transform midi data to nkt format
   amTrace("[MID->NKT] processing data ...\n");

   // reserve memory for all tracks
   uint32 error = 0;

   for(int i=0;i<nbOfTracks;++i){
        error = midiTrackDataToNkt(pMidiData, pNewSeq, i);
   }

//save
 if(saveSequence(pNewSeq,pOutFileName,bCompress)<0){
   amTrace("[MIDI2NKT] Fatal error, saving %s failed.\n", pOutFileName);
 }else{
   amTrace("[MIDI2NKT] Saved %s .\n", pOutFileName);
 }


 return pNewSeq;
}
