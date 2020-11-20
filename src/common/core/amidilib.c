
/**  Copyright 2007-2020 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#include "amidilib.h"
#include "mparser.h"
#include "timing/miditim.h"
#include "containers/list.h"
#include "mformats.h"
#include "fmio.h"
#include "midi_send.h"
#include "memory/endian.h"

/////////// formats
#include "midi.h"
#include "xmidi.h"
#include "dmus.h"
#include "nkt.h"

// devices
#include "roland.h"
#include "rol_ptch.h"
#include "config.h"

#include <mint/ostruct.h>
#include <stdio.h>

static const sAMIDI_version version = { AMIDI_MAJOR_VERSION, AMIDI_MINOR_VERSION, AMIDI_PATCHLEVEL };

#ifdef DEBUG_BUILD
static const uint8 outputFilename[] = "amidi.log";
#endif

//default configuration filename
static const uint8 configFilename[] = "amidi.cfg";

// memory callbacks


//

unsigned long begin;
unsigned long end;
long usp;

/* */ 
static const uint8 *g_arMidiDeviceTypeName[]={
  "Roland MT-32",       
  "Roland CM-32L/CM-64",
  "GS/GM",       
  "LA/GS mixed mode",
  "GM",
  "MT-32 GM emulation",
  "Yamaha XG GM mode"
};

const sAMIDI_version *amGetVersionInfo(void){
  return (const sAMIDI_version *)(&version); 
}

eMidiFileType amGetMidiDataType(void * const pMidiPtr)
{
  sMThd *pMidiInfo=0;
  amTrace((const uint8 *)"Checking header info... ");
  pMidiInfo=(sMThd *)pMidiPtr;
  
  /* check midi header */
  if(((pMidiInfo->id)==(ID_MTHD)&&(pMidiInfo->headLenght==6L)))
  {
     switch(pMidiInfo->format)
     {
  	 case T_MIDI0:
  	  /* Midi Format 0 detected */
  	 amTrace((const uint8*)"MIDI type 0 found\n");
     return T_MIDI0;
  	 break;

  	case T_MIDI1:
  	 /* Midi Format 1 detected */
  	 amTrace((const uint8*)"MIDI type 1 found\n");
     return T_MIDI1;
  	 break;

      case T_MIDI2:
  	/* Midi Format 2 detected */
  	amTrace((const uint8*)"MIDI type 2 found\n");
    return T_MIDI2;
  	break;
   };
  }

/* check XMIDI file */
if(amIsValidXmidiData(pMidiPtr))
{
  amTrace((const uint8*)"XMIDI file found.\n");
  return T_XMIDI;
}

/* Check MUS */
MUSheader_t *pMusHeader=(MUSheader_t *)pMidiPtr;

if(((pMusHeader->ID)>>8) == MUS_ID){
  amTrace((const uint8*)"Doom MUS found.\n");
  return T_MUS ;
}

//unsupported format  
 return T_UNSUPPORTED;
}

int16 amProcessMidiFileData(const char *filename, void *midiData, const uint32 dataSize, sSequence_t **ppSequence)
{
    (*ppSequence) = (sSequence_t *) amMallocEx( sizeof(sSequence_t), PREFER_TT);
 
    sSequence_t *sequence = *ppSequence;
    
    if(sequence==0)
    {
      amTrace((const uint8*)"Error: Cannot allocate memory for sequence.\n");
      printf( "Error: Cannot allocate memory for sequence.\n");
      return -1;
    }
   
   amMemSet(sequence, 0, sizeof(sSequence_t));

#ifdef EVENT_LINEAR_BUFFER
   const MemSize memSize = getGlobalConfig()->eventPoolSize * getGlobalConfig()->eventDataAllocatorSize;
   amTrace((const uint8 *)"amProcessMidiFileData() trying to allocate %d Kb\n",memSize/1024);

   if(createLinearBuffer(&(sequence->eventBuffer), memSize, PREFER_TT)<0){
       printf( "Error: Cannot allocate memory for sequence internal event buffer...\n");
       amFree(sequence);
       return -1;
   }
#endif

   const eMidiFileType midiType = amGetMidiDataType(midiData);
    
   if(midiType == T_INVALID)
   {
    /* not MIDI file, do nothing */
    amTrace((const uint8*)"It's not valid (X)MIDI file...\n");
    printf( "It's not valid MIDI file...\n");
    return -1;
   } 
   else if(midiType == T_UNSUPPORTED)
   {
    /* unsupported MIDI type format, do nothing*/
    amTrace((const uint8*)"Unsupported (X)MIDI file format...\n");
    printf( "Unsupported MIDI file format...\n");
    return -1; 
   }

   int16 iRetVal = 0;

   switch(midiType)
   {
        case T_MIDI0:
        {
          /* handle MIDI type 0 */
          const sMThd * const pMidiInfo = (sMThd *)midiData;
            
          if(pMidiInfo->nTracks != 1)
          {
            iRetVal=-1; /* invalid number of tracks, there can be only one! */
          } 
          else
          {
             /* OK! valid number of tracks */
             /* get time division for timing */

             const uint16 iTimeDivision = amGetTimeDivision(midiData);
              
             /* prepare our structure */
             sequence->ubNumTracks = pMidiInfo->nTracks;	/* one by default */

             /* Store time division for sequence, TODO: SMPTE handling */
             sequence->timeDivision = amDecodeTimeDivisionInfo(iTimeDivision);	/* PPQN */

             /* process track data, offset the start pointer a little to get directly to track data and decode MIDI events */
             midiData = (void *)((uint32)midiData + sizeof(sMThd));

             /* create one track list only */
             sequence->arTracks[0] = (sTrack_t *)amMallocEx(sizeof(sTrack_t),PREFER_TT);
             amMemSet(sequence->arTracks[0], 0, sizeof(sTrack_t));
             sequence->arTracks[0]->pTrkEventList = 0;

             midiData = processMidiTracks(midiData, midiType, ppSequence, &iRetVal);
          }

        } break;

        case T_MIDI1:
        {
          /* handle MIDI type 1 */
          /* several tracks, one sequence */
 
          const sMThd * const pMidiInfo = (sMThd *)midiData;
          const uint16 iTimeDivision = amGetTimeDivision(midiData);

          sequence->timeDivision = amDecodeTimeDivisionInfo(iTimeDivision);	/* PPQN */

          midiData=(void *)((uint32)midiData + sizeof(sMThd));
                	
          /* Store time division for sequence, TODO: SMPTE handling */
          sequence->ubNumTracks = pMidiInfo->nTracks;
	  
          for(uint16 i=0;i<pMidiInfo->nTracks;++i)
          {
            sequence->arTracks[i] = (sTrack_t *)amMallocEx(sizeof(sTrack_t),PREFER_TT);
            amMemSet(sequence->arTracks[i], 0, sizeof(sTrack_t));
          }
	  
          while ( midiData!=0 && iRetVal>0)
          {
            midiData = processMidiTracks(midiData, midiType, ppSequence, &iRetVal);
          }

        } break;

        case T_MIDI2:
        {
           /* handle MIDI type 2 */
           /* several tracks not tied to each other */
           const sMThd * const pMidiInfo = (sMThd *)midiData;

           const uint16 iNumTracks = pMidiInfo->nTracks;
           const uint16 iTimeDivision = amGetTimeDivision(midiData);

           sequence->timeDivision = amDecodeTimeDivisionInfo(iTimeDivision);	/* PPQN */

           midiData = (void *)((uint32)midiData + sizeof(sMThd));
		
           /* Store time division for sequence, TODO: SMPTE handling */
           sequence->ubNumTracks = pMidiInfo->nTracks;
	  
           for(uint16 i=0;i<pMidiInfo->nTracks;++i)
           {
             sequence->arTracks[i] = (sTrack_t *)amMallocEx(sizeof(sTrack_t),PREFER_TT);
             amMemSet(sequence->arTracks[i], 0, sizeof(sTrack_t));

             /* init event list */
             sequence->arTracks[i]->pTrkEventList=0;
          }
           
          while ( midiData!=0 && iRetVal>0)
          {
            midiData = processMidiTracks(midiData, midiType, ppSequence, &iRetVal);
          } 

        } break;
	
        case T_XMIDI:
        {
          /* processing (X)MIDI file, similar to MIDI type 2 in concept */
          const uint16 iNumTracks = amGetNbOfXmidiTracks(midiData);
          sequence->ubNumTracks = iNumTracks;
          
          if(iNumTracks == 0) 
          {
            printf("Error: Invalid number of tracks!\n");
            iRetVal = -1;
          }
          else
          {

            printf("XMIDI tracks to process: %d\n",iNumTracks);

            for(uint16 i=0;i<iNumTracks;++i)
            {
              sequence->arTracks[i] = (sTrack_t *)amMallocEx(sizeof(sTrack_t),PREFER_TT);
              amMemSet(sequence->arTracks[i], 0, sizeof(sTrack_t));

              /* init event list */
              sequence->arTracks[i]->pTrkEventList=0;
            }

            iRetVal = amProcessXmidiData(midiData, dataSize, ppSequence);

          }

        } break;

	      case T_RMID:
	      case T_SMF:
	      case T_XMF:
	      case T_SNG:
        {
          iRetVal = -1;
        } break;
  
        case T_NKT:
        {
          const sNktSeq * const nktSeq = (sNktSeq *)midiData;
          const uint16 iNumTracks = nktSeq->nbOfTracks;       //todo handle/setup replay
          iRetVal = -1;
        } break;

	    case T_MUS:
      {
          printf("Converting MUS to MIDI\n");
          uint8 *pOut = 0;

          // allocate 64kb working buffer for midi output
          pOut = (uint8 *)amMallocEx(64 * 1024,PREFER_TT);
          
          if(pOut)
          {
            char tempName[128] = {0};
            uint32 len = 0;

            // set midi output name
            if(filename)
            {
              char *pTempPtr = 0;
              int16 len = strlen(filename);
              strncpy(tempName, filename, 128);
              pTempPtr = strrchr(filename,'.');
              memcpy(pTempPtr+1,"mid",4);
            }

            Mus2Midi(midiData,(unsigned char *)pOut,tempName,&len);

            printf("Processing midi data..\n");
         
            /* the rest is like in MIDI type 0, handle MIDI type 0 */
            const sMThd * const pMidiInfo = (sMThd *)pOut;

            if(pMidiInfo->nTracks!=1)
            {
              /* invalid number of tracks, there can be only one! */
              amTrace("Invalid number of tracks\n");
              iRetVal = -1;
            }
            else
            {
              /* OK! valid number of tracks */
              /* get time division for timing */
              
              const uint16 iTimeDivision = amGetTimeDivision(pOut);
              sequence->ubNumTracks = pMidiInfo->nTracks; /* one by default */
              sequence->timeDivision = amDecodeTimeDivisionInfo(iTimeDivision); /* PPQN */

              /* process track data, offset the start pointer a little to get directly to track data and decode MIDI events */
              midiData = (void *)((uint32)pOut + sizeof(sMThd));
              const uint16 trackIdx = sequence->ubNumTracks-1;
              
              /* create one track list only */
              sequence->arTracks[trackIdx] = (sTrack_t *)amMallocEx(sizeof(sTrack_t),PREFER_TT);
              amMemSet(sequence->arTracks[trackIdx], 0, sizeof(sTrack_t));

              /* init event list */
              sequence->arTracks[trackIdx]->pTrkEventList=0;
              midiData = processMidiTracks(midiData, T_MIDI0, ppSequence, &iRetVal);
            }

          // free up working buffer
          amFree(pOut);

          }
          else
          {
              iRetVal=-1;
          }

	  } break;
	  
 }; //switch

 return iRetVal;
}

#ifndef IKBD_MIDI_SEND_DIRECT
static uint8 g_arMidiBuffer[MIDI_SENDBUFFER_SIZE];

/* Midi buffers system info */
static _IOREC g_sOldMidiBufferInfo;
static _IOREC *g_psMidiBufferInfo;
#endif

extern bool CON_LOG;
extern FILE *ofp;

int16 amInit(void)
{
    
#ifdef DEBUG_BUILD
 // init logger
 initDebug(outputFilename);
#endif 
  
  loadConfig(configFilename);

  //save configuration
  if(saveConfig(configFilename)>=0L){
    printf("Configuration saved sucessfully.\n");
  }else{
    printf("Error: Cannot save global configuration.\n");
    return -1;
  }
  
#ifndef IKBD_MIDI_SEND_DIRECT
  /* clear our new XBIOS buffer */
 uint32 usp=0L;
 amMemSet(g_arMidiBuffer,0,MIDI_SENDBUFFER_SIZE);

 usp=Super(0L);
 g_psMidiBufferInfo=(_IOREC*)Iorec(XB_DEV_MIDI);
		
 /* copy old MIDI buffer info */
 g_sOldMidiBufferInfo.ibuf=(*g_psMidiBufferInfo).ibuf;
 g_sOldMidiBufferInfo.ibufsiz=(*g_psMidiBufferInfo).ibufsiz;
 g_sOldMidiBufferInfo.ibufhd=(*g_psMidiBufferInfo).ibufhd;
 g_sOldMidiBufferInfo.ibuftl=(*g_psMidiBufferInfo).ibuftl;
 g_sOldMidiBufferInfo.ibuflow=(*g_psMidiBufferInfo).ibuflow;
 g_sOldMidiBufferInfo.ibufhi=(*g_psMidiBufferInfo).ibufhi;

 /* set up new MIDI buffer */
 (*g_psMidiBufferInfo).ibuf = (char *)g_arMidiBuffer;
 (*g_psMidiBufferInfo).ibufsiz = getGlobalConfig()->midiBufferSize;
 (*g_psMidiBufferInfo).ibufhd=0;	/* first byte index to write */
 (*g_psMidiBufferInfo).ibuftl=0;	/* first byte to read(remove) */
 (*g_psMidiBufferInfo).ibuflow=(uint16) MIDI_LWM;
 (*g_psMidiBufferInfo).ibufhi=(uint16) MIDI_HWM;
 SuperToUser(usp);
#endif

   // now depending on the connected device type and chosen operation mode
   // set appropriate channel
   // prepare device for receiving messages
   setupMidiDevice(getGlobalConfig()->connectedDeviceType,getGlobalConfig()->midiChannel);
   
 return 1;
}

void amDeinit(void){

#ifdef IKBD_MIDI_SEND_DIRECT
    // send content of midi buffer to device
   Supexec(flushMidiSendBuffer);
#endif

#ifndef IKBD_MIDI_SEND_DIRECT
  uint32 usp = Super(0L);
 
  /* restore standard MIDI buffer */
  (*g_psMidiBufferInfo).ibuf=g_sOldMidiBufferInfo.ibuf;
  (*g_psMidiBufferInfo).ibufsiz=g_sOldMidiBufferInfo.ibufsiz;
  (*g_psMidiBufferInfo).ibufhd=g_sOldMidiBufferInfo.ibufhd;
  (*g_psMidiBufferInfo).ibuftl=g_sOldMidiBufferInfo.ibuftl;
  (*g_psMidiBufferInfo).ibuflow=g_sOldMidiBufferInfo.ibuflow;
  (*g_psMidiBufferInfo).ibufhi=g_sOldMidiBufferInfo.ibufhi;
  SuperToUser(usp);
#endif

#ifdef DEBUG_BUILD
  deinitDebug();
#endif  
 /* end sequence */
}

void amGetDeviceInfoResponse(const uint8 channel)
{
  //TODO: rework it
  return;

  static uint8 getInfoSysEx[]={0xF0,ID_ROLAND,GS_DEVICE_ID,GS_MODEL_ID,0x7E,0x7F,0x06,0x01,0x00,0xF7};
  //uint8 getInfoSysEx[]={0xF0,0x41,0x10,0x42,0x7E,0x7F,0x06,0x01,0x00,0xF7};
  
  bool bTimeout=FALSE;

  /* calculate checksum */
  getInfoSysEx[5]=amCalcRolandChecksum(&getInfoSysEx[2],&getInfoSysEx[4]);
  getInfoSysEx[5]=channel;
  getInfoSysEx[8]=amCalcRolandChecksum(&getInfoSysEx[5],&getInfoSysEx[7]);  

#ifdef IKBD_MIDI_SEND_DIRECT
  for(int i=0;i<10;i++){
    MIDIsendBuffer[MIDIbytesToSend++]=getInfoSysEx[i];
  }
  Supexec(flushMidiSendBuffer);
#else
  /* request data */
    MIDI_SEND_DATA(10,(void *)getInfoSysEx); 
#endif    
//   bool bFlag=FALSE;
//   uint32 data=0;
  
//amGetTimeStamp(); // get current timestamp
	
//    /* get reply or there was timeout */
//    while((MIDI_DATA_READY&&(amGetTimeDelta()<getGlobalConfig()->midiConnectionTimeOut))) {
//	data = GET_MIDI_DATA;
//	begin=amGetTimeStamp(); // data received, reset timestamp
      
//	 if(data!=0){
	  
//	  if(bFlag==FALSE){
//		amTrace((const uint8*)"Received device info on ch: %d\t",channel);
// 	    bFlag=TRUE;
//	}
	
//		amTrace((const uint8*)"%x\t",(unsigned int)data);
//    }

//}

 if(bTimeout==TRUE) amTrace((const uint8*)"Timeout on ch: %d\t",channel);

}
/* gets info about connected devices via MIDI interface */
const int8 *amGetConnectedDeviceInfo(void){
  uint8 channel;
  
  /*  request on all channels */
  amTrace((const uint8*)"Quering connected MIDI device...\n");
  
  for(channel=0;channel<0x7f;channel++){
    amGetDeviceInfoResponse(channel);
   }

 return NULL;
}

const uint8 *amGetMidiDeviceTypeName(const eMidiDeviceType device){
 return g_arMidiDeviceTypeName[device];
}

#ifdef DEBUG_BUILD
/* variable quantity reading test */
void VLQtest(void){
/* VLQ test */
    uint32 val[]={0x00, 0x7F,0x8100,0xC000,0xFF7F,0x818000, 0xFFFF7F,0x81808000,0xC0808000,0xFFFFFF7F };
    uint32 iCounter;
    
    uint8 valsize;
    
    amTrace((const uint8*)"VLQ decoding test\n");
    
    for (iCounter=0;iCounter<10;iCounter++)   {
        uint8 *pValPtr=NULL;
		valsize=0;
		uint32 result=0;
 		
		pValPtr=(uint8 *)(&val[iCounter]);
        
		while((*pValPtr)==0x00){pValPtr++;}
        
		result = readVLQ(pValPtr,&valsize);
     	amTrace((const uint8*)"VLQ value:%x, decoded: %x, size: %d\n",(unsigned int)val[iCounter], (unsigned int)result, valsize );
    }
    /* End of VLQ test */
}
#endif

