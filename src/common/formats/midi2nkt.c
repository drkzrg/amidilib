
/**  Copyright 2007-2014 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#include "nkt.h"

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

U16 g_TD; //evil global!!!

void Nkt_CreateHeader(sNktHd* header, const sMThd *pMidiHeader, const BOOL bCompress){

    WriteInt(&header->id,ID_NKT);
    WriteInt(&header->NbOfBlocks, 0);
    WriteShort(&header->division, pMidiHeader->division);
    WriteShort(&header->bPacked, bCompress);
    WriteShort(&header->version, 1);
    g_TD=pMidiHeader->division;
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
#define OUT_BUFFER_SIZE 1024   // should be sufficient if there are no big SysEx messages
                              // TODO: make it configurable
typedef struct sBufferInfo{
 U8 buffer[OUT_BUFFER_SIZE];
 U32 bufPos;
 U32 blocks_written;
 U32 bytes_written;
 void *pCompWrkBuf; // working buffer used for compression
} sBufferInfo_t;


typedef struct sRunningStatus{
  U16 recallRS;
  U8 runningStatus;
} sRunningStatus_t;


U32 processNoteOff(U8 **pMidiData, sRunningStatus_t *rs, sBufferInfo_t* bufferInfo){
sNoteOff_t *pNoteOff=0;

if(rs->recallRS==0){
    /* save last running status */
    rs->runningStatus=*(*pMidiData);

    bufferInfo->buffer[bufferInfo->bufPos++]=*(*pMidiData);

    /* now we can recall former running status next time */
    rs->recallRS=1;

    (*pMidiData)++;
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


U32 processNoteOn(U8 **pMidiData, sRunningStatus_t *rs, sBufferInfo_t* bufferInfo){
sNoteOn_t *pNoteOn=0;

if(rs->recallRS==0){
  /* save last running status */
  rs->runningStatus=*(*pMidiData);
  bufferInfo->buffer[bufferInfo->bufPos++]=*(*pMidiData);

  /* now we can recall former running status next time */
  rs->recallRS=1;
  (*pMidiData)++;
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

U32 processNoteAft(U8 **pMidiData, sRunningStatus_t *rs, sBufferInfo_t* bufferInfo){
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


U32 processControllerEvent(U8 **pMidiData, sRunningStatus_t *rs, sBufferInfo_t* bufferInfo){
sController_t *pContrEv=0;

if(rs->recallRS==0){
    /* save last running status */
    rs->runningStatus=*(*pMidiData);
    bufferInfo->buffer[bufferInfo->bufPos++]=*(*pMidiData);

    /* now we can recall former running status next time */
    rs->recallRS=1;
    (*pMidiData)++;
    pContrEv=(sController_t *)(*pMidiData);
 }else{
    pContrEv=(sController_t *)(*pMidiData);
 }

 bufferInfo->buffer[bufferInfo->bufPos++]=pContrEv->controllerNb;
 bufferInfo->buffer[bufferInfo->bufPos++]=pContrEv->value;

 amTrace(" c: %d  v: %d\n",pContrEv->controllerNb, pContrEv->value);
 (*pMidiData)=(*pMidiData)+sizeof(sController_t);
}

U32 processProgramChange(U8 **pMidiData, sRunningStatus_t *rs, sBufferInfo_t* bufferInfo){
sProgramChange_t *pPC=0;

if(rs->recallRS==0){
  /* save last running status */
  rs->runningStatus=*(*pMidiData);
  bufferInfo->buffer[bufferInfo->bufPos++]=*(*pMidiData);

  /* now we can recall former running status next time */
  rs->recallRS=1;
  (*pMidiData)++;
  pPC=(sProgramChange_t *)(*pMidiData);
}else{
  pPC=(sProgramChange_t *)(*pMidiData);
}
 amTrace(" p: %d \n",pPC->programNb);
 bufferInfo->buffer[bufferInfo->bufPos++]=pPC->programNb;

(*pMidiData)=(*pMidiData) + sizeof(sProgramChange_t);
}

U32 processChannelAft(U8 **pMidiData, sRunningStatus_t *rs,sBufferInfo_t* bufferInfo){
sChannelAft_t *pChAft=0;

if(rs->recallRS==0){
  /* save last running status */
  rs->runningStatus=*(*pMidiData);
  bufferInfo->buffer[bufferInfo->bufPos++]=*(*pMidiData);

  /* now we can recall former running status next time */
  rs->recallRS=1;
  (*pMidiData)++;
  pChAft=(sChannelAft_t *)(*pMidiData);
}else{
  pChAft=(sChannelAft_t *)(*pMidiData);
}
 amTrace(" press: %d \n",pChAft->pressure);

 bufferInfo->buffer[bufferInfo->bufPos++]=pChAft->pressure;

(*pMidiData)=(*pMidiData)+sizeof(sChannelAft_t);
}

U32 processPitchBend(U8 **pMidiData, sRunningStatus_t *rs,sBufferInfo_t* bufferInfo){;
sPitchBend_t *pPitchBend=0;

if(rs->recallRS==0){
 /* save last running status */
 rs->runningStatus=*(*pMidiData);
 bufferInfo->buffer[bufferInfo->bufPos++]=*(*pMidiData);

 /* now we can recall former running status next time */
 rs->recallRS=1;
 (*pMidiData)++;
  pPitchBend=(sPitchBend_t *)(*pMidiData);
}else{
  pPitchBend=(sPitchBend_t *)(*pMidiData);
}

bufferInfo->buffer[bufferInfo->bufPos++]=pPitchBend->LSB;
bufferInfo->buffer[bufferInfo->bufPos++]=pPitchBend->MSB;

amTrace(" LSB: %d MSB: %d\n",pPitchBend->LSB,pPitchBend->MSB);

(*pMidiData)=(*pMidiData)+sizeof(sPitchBend_t);

}


#ifdef ENABLE_GEMDOS_IO
U32 processMetaEvent( U32 delta, U8 **pMidiData, S16 *fileHandle, sRunningStatus_t *rs, sBufferInfo_t* bufferInfo, BOOL *bEOT){
#else
U32 processMetaEvent( U32 delta, U8 **pMidiData, FILE **file, sRunningStatus_t *rs, sBufferInfo_t* bufferInfo, BOOL *bEOT){
#endif

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

     amTrace("Write End of Track, event delta 0\n");
     printf("Write End of Track, event delta 0\n");

     stBlock.blockSize=0;
     stBlock.msgType=NKT_END;

     //write block to file

     // write VLQ data
         U32 VLQdeltaTemp=0;
         S32 count=0;
         S32 bytesWritten=0;

         count=WriteVarLen((S32)delta, (U8 *)&VLQdeltaTemp);

#ifdef ENABLE_GEMDOS_IO
         bytesWritten=Fwrite(*fileHandle,count,&VLQdeltaTemp);

         if(bytesWritten!=count){
           amTrace("[WR] %ld bytes [W] %ld bytes\n",count, bytesWritten);
         }

         bufferInfo->bytes_written+=bytesWritten;

         bytesWritten=Fwrite(*fileHandle,sizeof(stBlock),&stBlock);

         if(bytesWritten!=count){
           amTrace("[WR] %ld bytes [W] %ld bytes\n",count, bytesWritten);
         }

         bufferInfo->bytes_written+=bytesWritten;

#else
         bufferInfo->bytes_written+=fwrite(&VLQdeltaTemp,count,1,*file);
         bufferInfo->bytes_written+=fwrite(&stBlock,sizeof(stBlock),1,*file);
#endif
         //no data to write

     ++(bufferInfo->blocks_written);
     *bEOT=TRUE;
    }break;
    case MT_SET_TEMPO:{

        stBlock.blockSize = 5 * sizeof(U32);   //U32 tempo value + 4*U32 values (25,50,100,200hz timesteps)

        stBlock.msgType=NKT_TEMPO_CHANGE;

        U8 ulVal[3]={0};   /* for retrieving set tempo info */
        U32 val1=0,val2=0,val3=0;
        amMemCpy(ulVal, (*pMidiData),metaLenght*sizeof(U8) );

        val1=ulVal[0],val2=ulVal[1],val3=ulVal[2];
        val1=(val1<<16)&0x00FF0000L;
        val2=(val2<<8)&0x0000FF00L;
        val3=(val3)&0x000000FFL;

        /* range: 0-8355711 ms, 24 bit value */
        val1=val1|val2|val3;
        amTrace((const U8*)"%lu ms per quarter-note\n", val1);

        // write VLQ delta
           U32 VLQdeltaTemp=0;
           S32 count=WriteVarLen((S32)delta, (U8 *)&VLQdeltaTemp);

#ifdef ENABLE_GEMDOS_IO
           S32 bytesWritten=0;

           bytesWritten = Fwrite(*fileHandle,count,&VLQdeltaTemp);
           bufferInfo->bytes_written += bytesWritten;

           if(bytesWritten!=count){
             amTrace("[WR] %ld bytes [W] %ld bytes\n",count, bytesWritten);
           }

           bytesWritten = Fwrite(*fileHandle,sizeof(stBlock),&stBlock);
           bufferInfo->bytes_written += bytesWritten;

           if(bytesWritten!=sizeof(stBlock)){
             amTrace("[WR] %ld bytes [W] %ld bytes\n",sizeof(stBlock), bytesWritten);
           }

           bytesWritten = Fwrite(*fileHandle,sizeof(U32),&val1);

           if(bytesWritten!=sizeof(U32)){
             amTrace("[WR] %ld bytes [W] %ld bytes\n",sizeof(U32), bytesWritten);
           }

           bufferInfo->bytes_written += bytesWritten;

#else
           bufferInfo->bytes_written += fwrite(&VLQdeltaTemp,count,1,*file);
           bufferInfo->bytes_written += fwrite(&stBlock,sizeof(stBlock),1,*file);
           bufferInfo->bytes_written += fwrite(&val1,sizeof(U32),1,*file);
#endif

           U32 precalc[NKT_UMAX]={0L};
           U32 td = g_TD ;
           U32 bpm = 60000000UL / val1;
           U32 tempPPU = bpm * td;

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
                        //assert(0);
                        amTrace((const U8*)"[Error] Invalid timer update value %d\n", i);
                    } break;
                };

           } //end for

           // write precalculated values
#ifdef ENABLE_GEMDOS_IO
           bytesWritten=Fwrite(*fileHandle,NKT_UMAX*sizeof(U32),&precalc);

           if(bytesWritten!=(NKT_UMAX*sizeof(U32))){
             amTrace("[WR] %ld bytes [W] %ld bytes\n",(NKT_UMAX*sizeof(U32)), bytesWritten);
           }

           bufferInfo->bytes_written+=bytesWritten;
#else
           bufferInfo->bytes_written+=fwrite(&precalc,NKT_UMAX*sizeof(U32),1,*file);
#endif


        ++(bufferInfo->blocks_written);
    }break;

    case MT_SMPTE_OFFSET:{}break;
    case MT_TIME_SIG:{}break;
    case MT_KEY_SIG:{}break;
    case MT_SEQ_SPEC:{}break;
 };

(*pMidiData)+=metaLenght;

}

U32 processSysex(U8 **pMidiData, sRunningStatus_t *rs, sBufferInfo_t* bufferInfo){
U32 ulCount=0L;
U8 *pDataPtr=0;

pDataPtr=(*pMidiData); //save SysEX start

while( (*(*pMidiData))!=EV_EOX){
    (*pMidiData)++;
    /* count Sysex msg data bytes */
    ++ulCount;
}

// copy data
amMemCpy(&bufferInfo->buffer[bufferInfo->bufPos],pDataPtr,ulCount);

}

//
#ifdef ENABLE_GEMDOS_IO
U32 processMidiEvent(const U32 delta, U8 **pCmd, sRunningStatus_t *rs, sBufferInfo_t* bufferInfo ,S16 *fileHandle, BOOL *bEOF)
#else
U32 processMidiEvent(const U32 delta, U8 **pCmd, sRunningStatus_t *rs, sBufferInfo_t* bufferInfo ,FILE** file, BOOL *bEOF)
#endif
{
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
               amTrace("delta: %lu NOTE OFF\t", delta);
               iError=processNoteOff(pCmd,rs, bufferInfo);
             break;
             case EV_NOTE_ON:
               amTrace("delta: %lu NOTE ON\t", delta);
               iError=processNoteOn(pCmd,rs, bufferInfo);
             break;
             case EV_NOTE_AFTERTOUCH:
               amTrace("delta: %lu NOTE AFT\t", delta);
               iError=processNoteAft(pCmd,rs, bufferInfo);
             break;
             case EV_CONTROLLER:
               amTrace("delta: %lu CONTROLLER\t", delta);
               iError=processControllerEvent(pCmd,rs, bufferInfo );
             break;
             case EV_PROGRAM_CHANGE:
               amTrace("delta: %lu PROGRAM CHANGE\t", delta);
               iError=processProgramChange(pCmd,rs, bufferInfo);
             break;
             case EV_CHANNEL_AFTERTOUCH:
                amTrace("delta: %lu NOTE AFT\t", delta);
               iError=processChannelAft(pCmd,rs, bufferInfo);
             break;
             case EV_PITCH_BEND:
              amTrace("delta: %lu PITCH BEND\t", delta);
               iError=processPitchBend(pCmd,rs, bufferInfo);
             break;
             case EV_META:
              amTrace("delta: %lu META\t", delta);
#ifdef ENABLE_GEMDOS_IO
              iError=processMetaEvent(delta, pCmd, fileHandle, rs, bufferInfo, bEOF);
#else
              iError=processMetaEvent(delta, pCmd, file, rs, bufferInfo, bEOF);
#endif

             break;
             case EV_SOX:                          	/* SySEX midi exclusive */
               amTrace("delta: %lu SYSEX\n", delta);
               rs->recallRS=0; 	                /* cancel out midi running status */
               iError=(S16)processSysex(pCmd,rs, bufferInfo);
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

#ifdef ENABLE_GEMDOS_IO

U32 midiTrackDataToFile(void *pMidiData, S16 *fileHandle, sBufferInfo_t *pBufInfo){

#else

U32 midiTrackDataToFile(void *pMidiData, FILE **file, sBufferInfo_t *pBufInfo){

#endif


/* process track data, offset the start pointer a little to get directly to track data and decode MIDI events */
sChunkHeader *pTrackHd=0;
U32 trackChunkSize=0;

void *startTrkPtr=(void *)(((U8 *)pMidiData)+sizeof(sMThd));
void *endTrkPtr=0;

pTrackHd=(sChunkHeader *)startTrkPtr;

if(pTrackHd->id!=ID_MTRK){
 printf( "Error: Cannot find MIDI track chunk. Exiting. \n");
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
 BOOL bEOF=FALSE;
 U8 *pCmd=(U8 *)startTrkPtr;
 U8 ubSize=0;
 sNktBlk stBlock;
 sRunningStatus_t rs;
 rs.runningStatus=0;
 rs.recallRS=0;

 pBufInfo->blocks_written=0;
 pBufInfo->bytes_written=0;
 pBufInfo->bufPos=0;

 while ( ((pCmd!=endTrkPtr)&&(bEOF!=TRUE)&&(iError>=0)) ){
  /* read delta time, pCmd should point to the command data */
  delta=readVLQ(pCmd,&ubSize);
  pCmd+=ubSize;

#ifdef ENABLE_GEMDOS_IO
  iError=processMidiEvent(delta, &pCmd, &rs, pBufInfo ,fileHandle,&bEOF);   // todo check error
#else
  iError=processMidiEvent(delta, &pCmd, &rs, pBufInfo ,&(*file),&bEOF);   // todo check error
#endif

  U32 currentDelta = readVLQ(pCmd,&ubSize);

  while((currentDelta==0)&&(pCmd!=endTrkPtr)&&(bEOF!=TRUE)&&(iError>=0)){
    pCmd+=ubSize;

#ifdef ENABLE_GEMDOS_IO
    iError=processMidiEvent(0,&pCmd, &rs, pBufInfo ,fileHandle,&bEOF);
#else
    iError=processMidiEvent(0,&pCmd, &rs, pBufInfo ,&(*file),&bEOF);
#endif

    currentDelta = readVLQ(pCmd,&ubSize);
  }

  // dump midi event block to memory
  if(pBufInfo->bufPos>0){

   stBlock.blockSize = pBufInfo->bufPos;
   stBlock.msgType = NKT_MIDIDATA;

#ifdef DEBUG_BUILD
      amTrace("[DATA] ");

      for(int j=0; j<stBlock.blockSize; j++){
       amTrace("[%x]",pBufInfo->buffer[j]);
      }

      amTrace(" [/DATA]\n");
#endif


      // write block to file

        // write VLQ delta
        S32 count=0;
        U32 VLQdeltaTemp=0;

        count=WriteVarLen((S32)delta, (U8 *)&VLQdeltaTemp);
#ifdef ENABLE_GEMDOS_IO
        amTrace("Write block: \t");

        S32 bytesWritten = Fwrite(*fileHandle,count,&VLQdeltaTemp);

        if(bytesWritten!=count){
          amTrace("[WR] %ld bytes [W] %ld bytes\n",count, bytesWritten);
        }

        pBufInfo->bytes_written+= bytesWritten;

        bytesWritten = Fwrite(*fileHandle, sizeof(sNktBlk), &stBlock);

        if(bytesWritten!=sizeof(sNktBlk)){
          amTrace("[WR] %ld bytes [W] %ld bytes\n",sizeof(sNktBlk), bytesWritten);
        }

        pBufInfo->bytes_written+= bytesWritten;

        bytesWritten = Fwrite(*fileHandle,stBlock.blockSize,&(pBufInfo->buffer[0]));;

        if(bytesWritten!=stBlock.blockSize){
          amTrace("[WR] %ld bytes [W] %ld bytes\n",stBlock.blockSize, bytesWritten);
        }

        pBufInfo->bytes_written+=bytesWritten;

#else
        pBufInfo->bytes_written+=fwrite(&VLQdeltaTemp,count,1,*file);

        amTrace("Write block: \t");
        pBufInfo->bytes_written+=fwrite(&stBlock, sizeof(sNktBlk), 1, *file);
        pBufInfo->bytes_written+=fwrite(&(pBufInfo->buffer[0]),stBlock.blockSize,1,*file);

#endif

      ++(pBufInfo->blocks_written);

      amTrace("delta: [%lu] type:[%d] block size:[%u] bytes \n",delta, stBlock.msgType, stBlock.blockSize);

      //clear buffer
      pBufInfo->bufPos=0;
      amMemSet(&(pBufInfo->buffer[0]),0L,OUT_BUFFER_SIZE);
  }

 } /*end of decode events loop */

 // OK
 return 0;
}

S32 Midi2Nkt(void *pMidiData, const U8 *pOutFileName, const BOOL bCompress){

BOOL error=FALSE;
sNktHd nktHead;
sBufferInfo_t BufferInfo;
BOOL bCompressionEnabled=bCompress;
S32 bytesWritten=0;

#ifdef ENABLE_GEMDOS_IO
S16 fileHandle=GDOS_INVALID_HANDLE;
#else
FILE* file=0;
#endif

amMemSet(&BufferInfo, 0L, sizeof(sBufferInfo_t));
amMemSet(&nktHead, 0L, sizeof(sNktHd));

if(pOutFileName==0 || strlen(pOutFileName)==0){
    amTrace("Midi2Nkt() output file name is empty.\n");
    printf("Midi2Nkt() output file name is empty. \n");

    return -1;
}

   // create file header
   Nkt_CreateHeader(&nktHead, (const sMThd *)pMidiData, FALSE);

#ifdef ENABLE_GEMDOS_IO
   //create file
   amTrace("[GEMDOS] Create file %s\n",pOutFileName);
   printf("[GEMDOS] Create file %s\n",pOutFileName);

   fileHandle=Fcreate(pOutFileName, 0);

   if(fileHandle>0){
       amTrace("[GEMDOS] Create file, gemdos handle: %d\n",fileHandle);

       bytesWritten=Fwrite(fileHandle,sizeof(sNktHd),&nktHead);

       if(bytesWritten!=sizeof(sNktHd)){
           amTrace("[WR] %ld bytes, [W]: %ld bytes",sizeof(sNktHd), bytesWritten);
       }

       BufferInfo.bytes_written+=bytesWritten;

   }else{
       amTrace("[GEMDOS] Error: %s.\n",getGemdosError(fileHandle));
       printf("[GEMDOS] Error: %s.\n",getGemdosError(fileHandle));
       return -1;
   }

#else
   file = fopen(pOutFileName, "w+b");
   BufferInfo.bytes_written+=fwrite(&nktHead, sizeof(sNktHd), 1, file);
#endif


if(bCompressionEnabled!=FALSE){

    BufferInfo.pCompWrkBuf=amMallocEx(LZO1X_MEM_COMPRESS,PREFER_TT);
    amMemSet(BufferInfo.pCompWrkBuf,0,LZO1X_MEM_COMPRESS);

    if(BufferInfo.pCompWrkBuf==0){
        amTrace(stderr,"Could't allocate buffer for compression. Turning off compression.\n");
        bCompressionEnabled=FALSE;
    }

    if(bCompressionEnabled!=FALSE){
     if(lzo_init()!=LZO_E_OK){
         amTrace(stderr,"Could't initialise LZO library. Turning off compression.\n");
     }else{
         amTrace(stderr,"[LZO] init ok.\n");
     }
 }

}

// process event and store them into file
amTrace(stderr,"[MID->NKT] processing data ...\n");
#ifdef ENABLE_GEMDOS_IO
    error = midiTrackDataToFile(pMidiData, &fileHandle, &BufferInfo);
#else
    error = midiTrackDataToFile(pMidiData, &file, &BufferInfo);
#endif

// optional step LZO compression
if(bCompressionEnabled!=FALSE){
    // read file beside the header

#ifdef ENABLE_GEMDOS_IO

    Fseek(sizeof(sNktHd),fileHandle,SEEK_SET); //file start
    S32 fStart=Fseek(0,fileHandle,SEEK_CUR); //get current pos

    Fseek(sizeof(sNktHd),fileHandle,SEEK_END); //file end
    S32 fEnd=Fseek(0,fileHandle,SEEK_CUR);

#else
    fseek(file, sizeof(sNktHd), SEEK_SET); // reset file pos

    S32 fStart = ftell(file);
    fseek(file, 0, SEEK_END); // set end
    S32 fEnd = ftell(file);
#endif

    U32 sizeOfBlock = fEnd - fStart;
    amTrace("[LZO] allocating %lu bytes for temporary buffer.\n",sizeOfBlock);

    U8 *pData=amMallocEx(sizeOfBlock,PREFER_TT);

#ifdef ENABLE_GEMDOS_IO
    Fseek(0,fileHandle,SEEK_SET);              //reset file pos
    Fseek(sizeof(sNktHd),fileHandle,SEEK_SET); //set file pos after header
#else
    fseek(file, 0, SEEK_SET); // reset file pos
    fseek(file, sizeof(sNktHd), SEEK_SET); // reset file pos
#endif

    if(pData!=NULL){

#ifdef ENABLE_GEMDOS_IO
        U32 read=Fread(fileHandle,sizeOfBlock,(void *)pData);

        if(read!=sizeOfBlock){
            amTrace("[GEMDOS] Read error, expected: %d, read: %d\n",sizeOfBlock,read);
        }

#else
        U32 read=fread((void *)pData,1,sizeOfBlock,file);
#endif
        amTrace("[LZO] read data %lu bytes.\n",read);

         if(read==sizeOfBlock){
            U32 nbBytesPacked=0;

            // allocate temp buffer
            U8 *pTempBuf=(U8 *)amMallocEx(sizeOfBlock/16,PREFER_TT);

            if(pTempBuf!=0){
             amMemSet(pTempBuf,0,sizeOfBlock/16);

             if(lzo1x_1_compress(pData,sizeOfBlock,pTempBuf,&nbBytesPacked,BufferInfo.pCompWrkBuf)==LZO_E_OK){

               if (nbBytesPacked >= sizeOfBlock){
                  amTrace("[LZO] Block contains incompressible data.\n");
               }else{

#ifdef DEBUG_BUILD
                  amTrace("[LZO] Block uncompressed: %lu compressed: %lu\n",sizeOfBlock, nbBytesPacked);
                  // buffer decompression test
                  amTrace("[LZO] Decompressing ...\n");

                  //lzo1x_decompress()
                  if(lzo1x_decompress_safe(pTempBuf,nbBytesPacked,pData,&sizeOfBlock,BufferInfo.pCompWrkBuf)!=LZO_E_OK){
                      amTrace("[LZO] Data decompression error.\n");
                  }else{
                      amTrace("[LZO] Block decompressed: %lu, packed: %lu\n",sizeOfBlock,nbBytesPacked);
                  }
#endif
#ifdef DEBUG_BUILD
                    amTrace("[CDATA] ");
                        for(int j=0;j<nbBytesPacked;j++){
                            amTrace("%x",pTempBuf[j]);
                        }
                    amTrace(" [/CDATA]\n");
#endif

#ifdef ENABLE_GEMDOS_IO

                    amTrace("[GEMDOS] Closing file handle : [%d] \n", fileHandle);

                    S16 err = Fclose(fileHandle);

                    if(err!=GDOS_OK){
                      amTrace("[GEMDOS] Error closing file handle : [%d] \n", fileHandle, getGemdosError(err));
                    }

#else
                    fclose(file);
#endif
                  nktHead.bytesPacked = nbBytesPacked;
                  nktHead.bPacked = TRUE;

#ifdef ENABLE_GEMDOS_IO
                  fileHandle=Fopen(pOutFileName,S_READWRITE);

                  if(fileHandle>0){
                    printf("[GEMDOS] File opened: %s, gemdos handle: %d. \n", pOutFileName, fileHandle);
                    amTrace("[GEMDOS] File opened: %s, gemdos handle: %d. \n", pOutFileName, fileHandle);

                    Fseek(sizeof(sNktHd),fileHandle,SEEK_SET);
                  }else{
                    printf("[GEMDOS] File %s open error. %s\n", pOutFileName, getGemdosError(fileHandle));
                    amTrace("[GEMDOS] File %s open error. %s\n", pOutFileName, getGemdosError(fileHandle));
                    return -1;
                  }

#else

                  file = fopen(pOutFileName, "w+b");
                  fseek(file, sizeof(sNktHd),SEEK_SET);

#endif

                  // store compressed block
#ifdef ENABLE_GEMDOS_IO

                  S32 writ=Fwrite(fileHandle,nbBytesPacked,pTempBuf);

#else
                  S32 writ=fwrite(pTempBuf,1,nbBytesPacked,file);
#endif

                  printf("[LZO] compressed data block: %lu bytes written\n",writ);
               }
             }else{
                 printf("[LZO] Compression error...\n");
             }

              amFree(pTempBuf);

            }else{
                amTrace("Couldn't allocate memory for temporary compression buffer.\n",BufferInfo.bufPos,nbBytesPacked);
            }

        }else{
           amTrace("[LZO] read data error...\n");
        }
       amFree(pData);
    }else{
      amTrace("[LZO] Error: allocating temporary buffer failed\n");
    }

    if(BufferInfo.pCompWrkBuf!=0){
      amFree(BufferInfo.pCompWrkBuf);
    }


} //end compression

// update header
    nktHead.NbOfBlocks = BufferInfo.blocks_written;
    nktHead.NbOfBytesData = BufferInfo.bytes_written;

#ifdef ENABLE_GEMDOS_IO
    amTrace("[GEMDOS] NKT header update\n");
    S32 offset=Fseek(0,fileHandle,SEEK_SET);

    if(offset<0){
      amTrace("[GEMDOS] Fseek error, %s\n",getGemdosError((S16)offset));
    }

    amTrace("[WR] blocks: %lu, data bytes:%lu\n", nktHead.NbOfBlocks,nktHead.NbOfBytesData);

    S32 writ=Fwrite(fileHandle, sizeof(sNktHd), &nktHead);

    if(writ!=sizeof(sNktHd)){
        amTrace("[GEMDOS] Write error to gemdos file handle [%d], expected: %d, written: %d\n", fileHandle,sizeof(sNktHd), writ);
    }

    amTrace("[GEMDOS] Closing file handle : [%d] \n", fileHandle);
    S16 err=Fclose(fileHandle);

    if(err!=GDOS_OK){
        amTrace("[GEMDOS] Error closing file handle : [%d] \n", fileHandle, getGemdosError(err));
    }

#else
    fseek(file,0,SEEK_SET);

    S32 writ=fwrite(&nktHead, 1, sizeof(sNktHd), file);
    amTrace("Writing nb of blocks %lu, bytes: %lu\n", nktHead.NbOfBlocks, nktHead.NbOfBytesData/1024, nktHead.NbOfBytesData);

    fclose(file);
#endif

    amTrace("Saved %d event blocks, %lu kb(%lu bytes) of data.\n", nktHead.NbOfBlocks, nktHead.NbOfBytesData/1024, nktHead.NbOfBytesData);
    printf("Saved %d event blocks, %lu kb(%lu bytes) of data.\n", nktHead.NbOfBlocks, nktHead.NbOfBytesData/1024, nktHead.NbOfBytesData);

}
