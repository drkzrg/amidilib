
#include "nkt.h"

#include "events.h"
#include "timing/miditim.h"
#include "memory/linalloc.h"
#include "midi.h"

void Nkt_CreateHeader(sNktHd* header, const sMThd *pMidiHeader, const BOOL bCompress){
    WriteInt(&header->id,ID_NKT);
    WriteInt(&header->NbOfBlocks, 0);
    WriteShort(&header->division, pMidiHeader->division);
    WriteShort(&header->bPacked, bCompress);
    WriteShort(&header->version, 1);
}


// from mparser.c
U8  isMidiChannelEvent(U8 byteEvent){
    if(( ((byteEvent&0xF0)>=0x80) && ((byteEvent&0xF0)<=0xE0)))
    {return 1;}
    else return 0;
}

U8  isMidiRTorSysex(U8 byteEvent){

    if( ((byteEvent>=(U8)0xF0)&&(byteEvent<=(U8)0xFF)) ){
      /* it is! */
        return (1);
    }
    else /*no, it's not! */
        return (0);
}

/* combine bytes function for pitch bend */
U16 combineBytes(U8 bFirst, U8 bSecond){
    U16 val;
    val = (U16)bSecond;
    val<<=7;
    val|=(U16)bFirst;
 return(val);
}

//

U32 processNoteOff(U8 **pMidiData, U16 *recallRS, U8 *runningStatus, U8 *tab, U32 *bufPos){
sNoteOff_t *pNoteOff=0;

if((*recallRS)==0){
    /* save last running status */
    (*runningStatus)=*(*pMidiData);

    /* now we can recall former running status next time */
    (*recallRS)=1;

    (*pMidiData)++;
    pNoteOff=(sNoteOff_t *)(*pMidiData);

    // TODO: copy NoteOff data
}else{
    /* recall last cmd status */
    /* and get parameters as usual */

    pNoteOff=(sNoteOff_t *)(*pMidiData);
    // TODO: copy NoteOff data
}

(*pMidiData)=(*pMidiData)+sizeof(sNoteOff_t);

}


U32 processNoteOn(U8 **pMidiData, U16 *recallRS,U8 *runningStatus, U8 *tab, U32 *bufPos){
sNoteOn_t *pNoteOn=0;

if((*recallRS)==0){
  /* save last running status */
  (*runningStatus)=*(*pMidiData);

  /* now we can recall former running status next time */
  (*recallRS)=1;
  (*pMidiData)++;

  // get channel from running status
  pNoteOn=(sNoteOn_t *)(*pMidiData);

  // TODO: handle data

 }else{
  pNoteOn=(sNoteOn_t *)(*pMidiData);

  // TODO: handle data

 }

(*pMidiData)=(*pMidiData)+sizeof(sNoteOn_t);

}

U32 processNoteAft(U8 **pMidiData, U16 *recallRS,U8 *runningStatus,U8 *tab, U32 *bufPos){;
  sNoteAft_t *pNoteAft=0;

  if((*recallRS)==0){
   /* save last running status */
   (*runningStatus)=*(*pMidiData);

   /* now we can recall former running status next time */
   (*recallRS)=1;
   (*pMidiData)++;

    pNoteAft=(sNoteAft_t *)(*pMidiData);
    // TODO: handle data

  }else{
    pNoteAft=(sNoteAft_t *)(*pMidiData);
    // TODO: handle data
  }

  (*pMidiData)=(*pMidiData)+sizeof(sNoteAft_t);
}


U32 processControllerEvent(U8 **pMidiData, U16 *recallRS,U8 *runningStatus,U8 *tab, U32 *bufPos){
sController_t *pContrEv=0;

if((*recallRS)==0){
    /* save last running status */
    (*runningStatus)=*(*pMidiData);
    /* now we can recall former running status next time */
    (*recallRS)=1;
    (*pMidiData)++;
    pContrEv=(sController_t *)(*pMidiData);
 }else{
    pContrEv=(sController_t *)(*pMidiData);
 }

 (*pMidiData)=(*pMidiData)+sizeof(sController_t);
}

U32 processProgramChange(U8 **pMidiData, U16 *recallRS,U8 *runningStatus,U8 *tab, U32 *bufPos){
sProgramChange_t *pPC=0;

if((*recallRS)==0){
  /* save last running status */
  (*runningStatus)=*(*pMidiData);

  /* now we can recall former running status next time */
  (*recallRS)=1;
  (*pMidiData)++;

  pPC=(sProgramChange_t *)(*pMidiData);

}else{
  pPC=(sProgramChange_t *)(*pMidiData);
}

(*pMidiData)=(*pMidiData) + sizeof(sProgramChange_t);
}

U32 processChannelAft(U8 **pMidiData, U16 *recallRS,U8 *runningStatus,U8 *tab, U32 *bufPos){
sChannelAft_t *pChAft=0;

if((*recallRS)==0){
  /* save last running status */
  (*runningStatus)=*(*pMidiData);

  /* now we can recall former running status next time */
  (*recallRS)=1;
  (*pMidiData)++;
  pChAft=(sChannelAft_t *)(*pMidiData);
}else{
  pChAft=(sChannelAft_t *)(*pMidiData);
}

(*pMidiData)=(*pMidiData)+sizeof(sChannelAft_t);
}

U32 processPitchBend(U8 **pMidiData, U16 *recallRS, U8 *runningStatus,U8 *tab, U32 *bufPos){;
sPitchBend_t *pPitchBend=0;

if((*recallRS)==0){
 /* save last running status */
 (*runningStatus)=*(*pMidiData);

 /* now we can recall former running status next time */
 (*recallRS)=1;
 (*pMidiData)++;
  pPitchBend=(sPitchBend_t *)(*pMidiData);
}else{
 pPitchBend=(sPitchBend_t *)(*pMidiData);
}

(*pMidiData)=(*pMidiData)+sizeof(sPitchBend_t);

}

U32 processMetaEvent( U32 delta, U8 **pMidiData, FILE **file, U16 *recallRS, U8 *runningStatus,U8 *tab, U32 *bufPos, BOOL *bEOT,  U32 *blocks_written, U32 *bytesWritten){
U8 size=0;
U32 metaLenght=0;
sNktBlk stBlock;

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

     printf("Write End of Track, event delta 0\n");

     stBlock.blockSize=0;
     stBlock.msgType=NKT_END;

     //write block to file
     if(file!=NULL){
         // write VLQ data
         U32 VLQdeltaTemp=0;
         S32 count=0;
         count=WriteVarLen((S32)delta, (U8 *)&VLQdeltaTemp);

         *bytesWritten+=fwrite(&VLQdeltaTemp,count,1,*file);
         *bytesWritten+=fwrite(&stBlock,sizeof(stBlock),1,*file);
         //no data to write
     }
     ++(*blocks_written);
     *bEOT=TRUE;
    }break;
    case MT_SET_TEMPO:{
        stBlock.blockSize=0;
        stBlock.msgType=NKT_TEMPO_CHANGE;

        U8 ulVal[3]={0};   /* for retrieving set tempo info */
        U32 val1,val2,val3;
        amMemCpy(ulVal, (*pMidiData),metaLenght*sizeof(U8) );

        val1=ulVal[0],val2=ulVal[1],val3=ulVal[2];
        val1=(val1<<16)&0x00FF0000L;
        val2=(val2<<8)&0x0000FF00L;
        val3=(val3)&0x000000FFL;

        /* range: 0-8355711 ms, 24 bit value */
        val1=val1|val2|val3;

        ++(*blocks_written);
        amTrace((const U8*)"%lu ms per quarter-note\n", val1);

    }break;
    case MT_SMPTE_OFFSET:{}break;
    case MT_TIME_SIG:{}break;
    case MT_KEY_SIG:{}break;
    case MT_SEQ_SPEC:{}break;
 };

(*pMidiData)+=metaLenght;

}

U32 processSysex(U8 **pMidiData, U16 *recallRS, U8 *runningStatus,U8 *tab, U32 *bufPos){;
U32 ulCount=0L;
U8 *pDataPtr=0;

pDataPtr=(*pMidiData); //save SysEX start

while( (*(*pMidiData))!=EV_EOX){
    (*pMidiData)++;
    /* count Sysex msg data bytes */
    ++ulCount;
}

// copy ulCount bytes from  pDataPtr
// TODO:

}

//

U32 midiTrackDataToFile(void *pMidiData, FILE **file, U32 *blocks_written, U32 *bytes_written){

U8 tempBuffer[32 * 1024]={0};

/* process track data, offset the start pointer a little to get directly to track data and decode MIDI events */
sChunkHeader *pTrackHd=0;
U32 bufPos=0;

U32 trackChunkSize=0;
void *startTrkPtr=(void *)(((U8 *)pMidiData)+sizeof(sMThd));
void *endTrkPtr=0;

pTrackHd=(sChunkHeader *)startTrkPtr;

if(pTrackHd->id!=ID_MTRK){
 printf("Error: Cannot find MIDI track chunk. Exiting. \n");
 amTrace("Error: Cannot find MIDI track chunk. Exiting. \n");
 return 1;
};

trackChunkSize=pTrackHd->headLenght;

// adjust to track start
startTrkPtr=(void *)( ((U8 *)pTrackHd) + sizeof(sChunkHeader));
endTrkPtr=(void *)((U8*)pTrackHd + trackChunkSize);

 // process track events
 U8 usSwitch=0;
 U8 lastRunningStatus=0;
 U16 recallStatus=0;
 U32 delta=0L;
 S32 iError=0;
 BOOL bEOF=FALSE;

 U8 *pCmd=(U8 *)startTrkPtr;
 U8 ubSize=0;
 U32 currentDelta=0;
 sNktBlk stBlock;

 while ( ((pCmd!=endTrkPtr)&&(bEOF!=TRUE)&&(iError>=0)) ){
  /* read delta time, pCmd should point to the command data */
  delta = readVLQ(pCmd,&ubSize);
  if(delta==currentDelta){
      //////////////////////////////////////////////////////////////////////
      delta=readVLQ(pCmd,&ubSize);

      pCmd+=ubSize;

      /* handling of running status */
      /* if byte is not from 0x08-0x0E range then recall last running status AND set recallStatus = 1 */
      /* else set recallStatus = 0 and do nothing special */
      ubSize=(*pCmd);

      if( (!(isMidiChannelEvent(ubSize))&&(recallStatus==1)&&(!(isMidiRTorSysex(ubSize))))){

        /* recall last cmd byte */
        usSwitch = lastRunningStatus;
        usSwitch = ((usSwitch>>4)&0x0F);

       }else{
        /* check if the new cmd is the system one */
          recallStatus=0;

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
            iError=processNoteOff(&pCmd,&recallStatus,&lastRunningStatus,tempBuffer,&bufPos);
          break;
          case EV_NOTE_ON:
            amTrace("delta: %lu NOTE ON\n", delta);
            iError=processNoteOn(&pCmd,&recallStatus,&lastRunningStatus,tempBuffer,&bufPos);
          break;
          case EV_NOTE_AFTERTOUCH:
            amTrace("delta: %lu NOTE AFT\n", delta);
            iError=processNoteAft(&pCmd,&recallStatus,&lastRunningStatus,tempBuffer,&bufPos);
          break;
          case EV_CONTROLLER:
            amTrace("delta: %lu CONTROLLER\n", delta);
            iError=processControllerEvent(&pCmd,&recallStatus,&lastRunningStatus,tempBuffer,&bufPos );
          break;
          case EV_PROGRAM_CHANGE:
            amTrace("delta: %lu PROGRAM CHANGE\n", delta);
            iError=processProgramChange(&pCmd,&recallStatus,&lastRunningStatus,tempBuffer,&bufPos);
          break;
          case EV_CHANNEL_AFTERTOUCH:
             amTrace("delta: %lu NOTE AFT\n", delta);
            iError=processChannelAft(&pCmd,&recallStatus,&lastRunningStatus,tempBuffer,&bufPos);
          break;
          case EV_PITCH_BEND:
           amTrace("delta: %lu PITCH BEND\n", delta);
            iError=processPitchBend(&pCmd,&recallStatus,&lastRunningStatus,tempBuffer,&bufPos);
          break;
          case EV_META:
           amTrace("delta: %lu META\n", delta);
            iError=processMetaEvent(delta, &pCmd,&(*file), &recallStatus,&lastRunningStatus,tempBuffer,&bufPos,&bEOF, blocks_written, bytes_written);
          break;
          case EV_SOX:                          	/* SySEX midi exclusive */
            amTrace("delta: %lu SYSEX\n", delta);
            recallStatus=0; 	                /* cancel out midi running status */
            iError=(S16)processSysex(&pCmd,&recallStatus,&lastRunningStatus,tempBuffer,&bufPos);
          break;
          case SC_MTCQF:
            amTrace("delta: %lu SC_MTCQF\n", delta);
            recallStatus=0;                        /* Midi time code quarter frame, 1 byte */
            amTrace((const U8*)"Event: System common MIDI time code qt frame\n");
            pCmd++;
            pCmd++;
          break;
        case SC_SONG_POS_PTR:
            amTrace((const U8*)"Event: System common Song position pointer\n");
            recallStatus=0;                      /* Song position pointer, 2 data bytes */
            pCmd++;
            pCmd++;
            pCmd++;
          break;
          case SC_SONG_SELECT:              /* Song select 0-127, 1 data byte*/
            amTrace((const U8*)"Event: System common Song select\n");
            recallStatus=0;
            pCmd++;
            pCmd++;
          break;
          case SC_UNDEF1:                   /* undefined */
          case SC_UNDEF2:                   /* undefined */
            amTrace((const U8*)"Event: System common not defined.\n");
            recallStatus=0;
            pCmd++;
          break;
          case SC_TUNE_REQUEST:             /* tune request, no data bytes */
            amTrace((const U8*)"Event: System tune request.\n");
            recallStatus=0;
            pCmd++;
          break;
          default:{
            amTrace((const U8*)"Event: Unknown type: %d\n",(*pCmd));
            /* unknown event, do nothing or maybe throw error? */
          }break;
        } //end switch


  } //end delta == 0

  // dump midi event block to memory
  if(bufPos>0){
      stBlock.blockSize=bufPos;

      amTrace("[DATA] ");

          for(int j=0;j<bufPos;j++){
              amTrace("0x%x ",tempBuffer[j]);
          }
      amTrace(" [/DATA]\n");

      //write block to file

      if(file!=NULL){
        // write VLQ delta
        S32 count=0;
        U32 VLQdeltaTemp=0;

        count=WriteVarLen((S32)currentDelta, (U8 *)&VLQdeltaTemp);
        *bytesWritten+=fwrite(&VLQdeltaTemp,count,1,*file);

        amTrace("Write block size %d\n",stBlock.blockSize);
        *bytes_written+=fwrite(&stBlock, sizeof(sNktBlk), 1, *file);
        *bytes_written+=fwrite(&tempBuffer[0],stBlock.blockSize,1,*file);
      }
      ++(*blocks_written);

      amTrace("delta [%lu] type:[%d] size:[%u] bytes \n",currentDelta, stBlock.msgType, stBlock.blockSize);

      //clear buffer
      bufDataSize=0;
      bufPos=0;
      amMemSet(tempBuffer,0,32 * 1024);
  }





 } /*end of decode events loop */

 // OK
 return 0;
}

S32 Midi2Nkt(void *pMidiData, const U8 *pOutFileName, const BOOL bCompress){
U32 bytes_written = 0;
U32 blocks_written = 0;
BOOL error=FALSE;
FILE* file=0;
sNktHd nktHead;

if(pOutFileName){
   // create file header
   amTrace("Writing NKT file to: %s\n",pOutFileName);
   Nkt_CreateHeader(&nktHead, (const sMThd *)pMidiData, bCompress);

   file = fopen(pOutFileName, "wb");
   bytes_written+=fwrite(&nktHead, sizeof(sNktHd), 1, file);
}

// process event and store them into file
error = midiTrackDataToFile(pMidiData, &file, &blocks_written,&bytes_written);

if(file){
   fseek(file, 0, SEEK_SET);
   // update header
   fwrite(&nktHead, sizeof(sNktHd), 1, file);

   fclose(file); file=0;
   amTrace("Stored %d event blocks, %lu kb(%lu bytes) of data.\n",nktHead.NbOfBlocks,nktHead.NbOfBytesData/1024,nktHead.NbOfBytesData);
 }

}
