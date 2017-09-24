
/**  Copyright 2007-2010 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#include "amidilib.h"
#include "mparser.h"
#include "timing/miditim.h"
#include "list/list.h"
#include "mformats.h"
#include "fmio.h"
#include "midi_send.h"

// formats
#include "midi.h"
#include "xmidi.h"
#include "dmus.h"
//

#include "roland.h"
#include "rol_ptch.h"
#include "config.h"

#include <mint/ostruct.h>

static const sAMIDI_version version = { AMIDI_MAJOR_VERSION, AMIDI_MINOR_VERSION, AMIDI_PATCHLEVEL };

#ifdef DEBUG_BUILD
static const U8 outputFilename[] = "amidi.log";
#endif

//default configuration filename
static const U8 configFilename[] = "amidi.cfg";

unsigned long begin;
unsigned long end;
long usp;

/* */ 
static const U8 *g_arMidiDeviceTypeName[]={
  "Roland MT-32",       
  "Roland CM-32L",   
  "GS/GM",       
  "LA/GS mixed mode",
  "MT-32 GM emulation",
  "Yamaha XG GM mode"
};

const sAMIDI_version *am_getVersionInfo(void){
  return (const sAMIDI_version *)(&version); 
}

S16 am_getHeaderInfo(const void *pMidiPtr){
    sMThd *pMidiInfo=0;
    amTrace((const U8 *)"Checking header info... ");
    pMidiInfo=(sMThd *)pMidiPtr;
  
/* check midi header */
if(((pMidiInfo->id)==(ID_MTHD)&&(pMidiInfo->headLenght==6L))){
        switch(pMidiInfo->format){
	 case T_MIDI0:
	  /* Midi Format 0 detected */
	  amTrace((const U8*)"MIDI type 0 found\n");
      return T_MIDI0;
	 break;

	case T_MIDI1:
	 /* Midi Format 1 detected */
	 amTrace((const U8*)"MIDI type 1 found\n");
     
     return T_MIDI1;
	 break;

    case T_MIDI2:
	/* Midi Format 2 detected */
	amTrace((const U8*)"MIDI type 2 found\n");
      return T_MIDI2;
	break;
   };
}else if ((pMidiInfo->id==ID_FORM)||(pMidiInfo->id==ID_CAT)){
      /* possible XMIDI*/
      amTrace((const U8*)"XMIDI file possibly..\n");
      return T_XMIDI;
}else{
     MUSheader_t *pMusHeader=(MUSheader_t *)pMidiPtr;

     if(((pMusHeader->ID)>>8)==MUS_ID){
      amTrace((const U8*)"Doom MUS found.\n");
      return T_MUS ;
     }
}
//unsupported format  
 return(-1);
}

S16 am_handleMIDIfile(const char *pFileName,void *pMidiPtr, U32 lenght, sSequence_t **pSequence){
    S16 iNumTracks=0;
    S16 iError=0;
    U16 iTimeDivision=0;
    void *startPtr=pMidiPtr;
    
    (*pSequence)=0;
    (*pSequence)=(sSequence_t *) amMallocEx( sizeof(sSequence_t), PREFER_TT);
    
    if((*pSequence)==0){
      amTrace((const U8*)"Error: Cannot allocate memory for sequence.\n");
      printf( "Error: Cannot allocate memory for sequence.\n");
      return -1;
    }
   
   amMemSet((*pSequence),0,sizeof(sSequence_t));
   (*pSequence)->ubActiveTrack=0;
    
   tMEMSIZE memSize=getGlobalConfig()->eventPoolSize*getGlobalConfig()->eventDataAllocatorSize;
   amTrace((const U8 *)"am_handleMIDIfile() trying to allocate %d Kb\n",memSize/1024);

#ifdef EVENT_LINEAR_BUFFER
   if(createLinearBuffer(&((*pSequence)->eventBuffer), memSize, PREFER_TT)<0){
       printf( "Error: Cannot allocate memory for sequence internal event buffer...\n");
       amFree((*pSequence));
       return -1;
   }
#endif

   int iRet=0;
   iRet=am_getHeaderInfo(pMidiPtr);
    
   if(iRet==-1){
    /* not MIDI file, do nothing */
    amTrace((const U8*)"It's not valid MIDI file...\n");
    printf( "It's not valid MIDI file...\n");
    return -1;
   } else if(iRet==-2){
    /* unsupported MIDI type format, do nothing*/
    amTrace((const U8*)"Unsupported MIDI file format...\n");
    printf( "Unsupported MIDI file format...\n");
   return -1; 
   }

    switch(iRet){
        case T_MIDI0:{
            /* handle MIDI type 0 */
            iNumTracks=am_getNbOfTracks(pMidiPtr,T_MIDI0);

            if(iNumTracks!=1){
                return(-1);
            } /* invalid number of tracks, there can be only one! */
            else{
               /* init sequence table */
                for(int iLoop=0;iLoop<AMIDI_MAX_TRACKS;iLoop++){
		 
                /* we will allocate needed track tables when appropriate */
                (*pSequence)->arTracks[iLoop]=NULL;
            }

            /* prepare our structure */
            (*pSequence)->ubNumTracks=iNumTracks;	/* one by default */
		 
            /* OK! valid number of tracks */
            /* get time division for timing */

            /* Store time division for sequence, TODO: SMPTE handling */
            iTimeDivision = am_getTimeDivision(pMidiPtr);
            (*pSequence)->timeDivision=am_decodeTimeDivisionInfo(iTimeDivision);	/* PPQN */

            /* process track data, offset the start pointer a little to get directly to track data and decode MIDI events */
            startPtr=(void *)((U32)startPtr+sizeof(sMThd));

            /* create one track list only */
            (*pSequence)->arTracks[0] = (sTrack_t *)amMallocEx(sizeof(sTrack_t),PREFER_TT);
            amMemSet((*pSequence)->arTracks[0],0,sizeof(sTrack_t));
		 		    
            /* init event list */
            (*pSequence)->arTracks[0]->pTrkEventList=0;
		  
		   while (startPtr!=0){

               /* Pointer to midi data,
                 type of midi to preprocess,
                 number of tracks,
                 pointer to the structure in which track data will be dumped (or not).
                */

                startPtr=processMidiTrackData(startPtr,T_MIDI0,1, pSequence,&iError);
                if(iError<0)return iError;
            }
         }
         return(0);
        }
        break;

        case T_MIDI1:{
         /* handle MIDI type 1 */
        /* several tracks, one sequence */
        /* prepare our structure */
        iNumTracks=am_getNbOfTracks(pMidiPtr,T_MIDI1);
	  
        /* init sequence table */
        for(int iLoop=0;iLoop<AMIDI_MAX_TRACKS;iLoop++){
            /* we will allocate needed track tables when appropriate */
            (*pSequence)->arTracks[iLoop]=NULL;
        }
	  
        iTimeDivision = am_getTimeDivision(pMidiPtr);
        (*pSequence)->timeDivision=am_decodeTimeDivisionInfo(iTimeDivision);	/* PPQN */

        startPtr=(void *)((U32)startPtr+sizeof(sMThd));
                	
        /* Store time division for sequence, TODO: SMPTE handling */
        (*pSequence)->ubNumTracks=iNumTracks;
	  
        /* create one track list only */
        for(int i=0;i<iNumTracks;i++){
            (*pSequence)->arTracks[i] = (sTrack_t *)amMallocEx(sizeof(sTrack_t),PREFER_TT);
            amMemSet((*pSequence)->arTracks[i],0,sizeof(sTrack_t));
            /* init event list */            
        }
	  
          while (startPtr!=0){
            startPtr=processMidiTrackData(startPtr,T_MIDI1, iNumTracks, pSequence,&iError);
            if(iError<0)return iError;
          }
         return(0);
        }
        break;

        case T_MIDI2:{
            /* handle MIDI type 2 */
            /* several tracks not tied to each others tracks */
            /* init sequence table */
            for(int iLoop=0;iLoop<AMIDI_MAX_TRACKS;iLoop++){
                /* we will allocate needed track tables when appropriate */
                (*pSequence)->arTracks[iLoop]=NULL;
        }
	  
        iNumTracks=am_getNbOfTracks(pMidiPtr,T_MIDI2);
        iTimeDivision = am_getTimeDivision(pMidiPtr);
        (*pSequence)->timeDivision=am_decodeTimeDivisionInfo(iTimeDivision);	/* PPQN */

        startPtr=(void *)((U32)startPtr+sizeof(sMThd));
		
        /* Store time division for sequence, TODO: SMPTE handling */
        (*pSequence)->ubNumTracks=iNumTracks;
	  
        /* create one track list only */
        for(int i=0;i<iNumTracks;i++){
	    (*pSequence)->arTracks[i] = (sTrack_t *)amMallocEx(sizeof(sTrack_t),PREFER_TT);
        amMemSet((*pSequence)->arTracks[i],0,sizeof(sTrack_t));

        /* init event list */
	    (*pSequence)->arTracks[i]->pTrkEventList=0;

	  }
           
       while (startPtr!=0){
         startPtr=processMidiTrackData(startPtr,T_MIDI2,iNumTracks,pSequence,&iError);
         if(iError<0)return iError;
        }
             return(0);
        }break;
	case T_XMIDI:{
         /* handle XMIDI */
         iNumTracks=am_getNbOfTracks(pMidiPtr,T_XMIDI);
         iTimeDivision = am_getTimeDivision(pMidiPtr);

         /* processing (X)MIDI file */
         /* TODO: handle + process */

         return(-1); /*xmidi isn't handled yet*/
    } break;

	case T_RMID:{return(-1);}break; 
	case T_SMF:{return(-1);}break;
	case T_XMF:{return(-1);}break;
	case T_SNG:{return(-1);}break;
    case T_NKT:{
            //todo handle/setup replay
        return(-1);
    }break;
	case T_MUS:{

      printf("Converting MUS to MIDI\n");

      U8 *pOut=0;
      char tempName[128]={0};
      U32 len=0;


      // allocate 64kb working buffer for midi output
      pOut = (U8 *)amMallocEx(64*1024,PREFER_TT);

     // set midi output name
     if(pFileName){
	     char *pTempPtr=0;

         strncpy(tempName,pFileName,strlen(pFileName));
         pTempPtr=strrchr(tempName,'.');
         memcpy(pTempPtr+1,"mid",4);
     }

      Mus2Midi(pMidiPtr,(unsigned char *)pOut,tempName,&len);
      printf("Processing midi data..\n");
        // the rest is like in MIDI type 0
        /* handle MIDI type 0 */
            iNumTracks=am_getNbOfTracks(pOut,T_MIDI0);

            if(iNumTracks!=1){
                /* invalid number of tracks, there can be only one! */
                amTrace("Invalid number of tracks\n");
                return(-1);
            }else{

            /* init sequence table */
            for(int iLoop=0;iLoop<AMIDI_MAX_TRACKS;iLoop++){
                /* we will allocate needed track tables when appropriate */
                (*pSequence)->arTracks[iLoop]=NULL;
            }

            /* prepare our structure */
            (*pSequence)->ubNumTracks=iNumTracks;	/* one by default */
		 
            /* OK! valid number of tracks */
            /* get time division for timing */
            iTimeDivision = am_getTimeDivision(pOut);
            (*pSequence)->timeDivision=am_decodeTimeDivisionInfo(iTimeDivision);	/* PPQN */

            /* process track data, offset the start pointer a little to get directly to track data and decode MIDI events */
            startPtr=(void *)((U32)pOut+sizeof(sMThd));

           /* create one track list only */
            (*pSequence)->arTracks[0] = (sTrack_t *)amMallocEx(sizeof(sTrack_t),PREFER_TT);
            
            amMemSet((*pSequence)->arTracks[0],0,sizeof(sTrack_t));

            /* init event list */
            (*pSequence)->arTracks[0]->pTrkEventList=0;
		  
		   while (startPtr!=0){
                startPtr=processMidiTrackData(startPtr,T_MIDI0,1, pSequence,&iError);
                if(iError<0)
                    return iError;
           }
          }
          // free up working buffer
          amFree(pOut);

	  return(0);
	  
	}break;
	
	default:{
	  /* unknown error, do nothing */
	  amTrace((const U8*)"Unknown error.\n");
	  printf( "Unknown error ...\n");
        
	  return(-1);
	}
	/* unsupported file type */
 }
 return(-1);
}

//TODO: rework interface or remove this function at all
S16 am_getNbOfTracks(void *pMidiPtr, S16 type){
    switch(type){
     case T_MIDI0:
     case T_MIDI1:
     case T_MIDI2:{
        sMThd *pMidiInfo=0;
        pMidiInfo=(sMThd *)pMidiPtr;
        
	/* check midi header */
        if(((pMidiInfo->id)==(ID_MTHD)&&(pMidiInfo->headLenght==6L))){
	  return (pMidiInfo->nTracks);
        }
     }
     break;

     case T_XMIDI:{
        /*TODO: ! not implemented */
    //sIFF_Chunk *pXmidiInfo=0;
	
	
	

	return -1;
     }
     case T_RMID:{
     return -1;/*TODO: ! not implemented */
     }break;
     case T_SMF:{
      return -1;/*TODO: ! not implemented */
     }break;
     case T_XMF:{
      return -1;/*TODO: ! not implemented */
     } break;
     
     case T_SNG:{
      return -1;/*TODO: ! not implemented */
    }break;
     case T_MUS:
       return -1;/*TODO: ! not implemented */
     case T_NKT:

        return 1;
     break;
     default:
       return -1;/*TODO: ! not implemented */
    }
return -1;
}

#ifndef IKBD_MIDI_SEND_DIRECT
static U8 g_arMidiBuffer[MIDI_SENDBUFFER_SIZE];

/* Midi buffers system info */
static _IOREC g_sOldMidiBufferInfo;
static _IOREC *g_psMidiBufferInfo;
#endif

extern BOOL CON_LOG;
extern FILE *ofp;

S16 am_init(){
    
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
 U32 usp=0L;
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
 (*g_psMidiBufferInfo).ibuflow=(U16) MIDI_LWM;
 (*g_psMidiBufferInfo).ibufhi=(U16) MIDI_HWM;
 SuperToUser(usp);
#endif

   // now depending on the connected device type and chosen operation mode
   // set appropriate channel
   // prepare device for receiving messages
   setupMidiDevice(getGlobalConfig()->connectedDeviceType,getGlobalConfig()->midiChannel);
   
 return 1;
}

void am_deinit(){

#ifdef IKBD_MIDI_SEND_DIRECT
    // send content of midi buffer to device
   Supexec(flushMidiSendBuffer);
#endif

#ifndef IKBD_MIDI_SEND_DIRECT
  U32 usp=Super(0L);
 
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

void getDeviceInfoResponse(U8 channel){
  //TODO: rework it
  return;

  static U8 getInfoSysEx[]={0xF0,ID_ROLAND,GS_DEVICE_ID,GS_MODEL_ID,0x7E,0x7F,0x06,0x01,0x00,0xF7};
  //U8 getInfoSysEx[]={0xF0,0x41,0x10,0x42,0x7E,0x7F,0x06,0x01,0x00,0xF7};
  
  BOOL bTimeout=FALSE;

  /* calculate checksum */
  getInfoSysEx[5]=am_calcRolandChecksum(&getInfoSysEx[2],&getInfoSysEx[4]);
  getInfoSysEx[5]=channel;
  getInfoSysEx[8]=am_calcRolandChecksum(&getInfoSysEx[5],&getInfoSysEx[7]);  

#ifdef IKBD_MIDI_SEND_DIRECT
  for(int i=0;i<10;i++){
    MIDIsendBuffer[MIDIbytesToSend++]=getInfoSysEx[i];
  }
  Supexec(flushMidiSendBuffer);
#else
  /* request data */
    MIDI_SEND_DATA(10,(void *)getInfoSysEx); 
#endif    
//   BOOL bFlag=FALSE;
//   U32 data=0;
  
//getTimeStamp(); // get current timestamp
	
//    /* get reply or there was timeout */
//    while((MIDI_DATA_READY&&(getTimeDelta()<getGlobalConfig()->midiConnectionTimeOut))) {
//	data = GET_MIDI_DATA;
//	begin=getTimeStamp(); // data received, reset timestamp
      
//	 if(data!=0){
	  
//	  if(bFlag==FALSE){
//		amTrace((const U8*)"Received device info on ch: %d\t",channel);
// 	    bFlag=TRUE;
//	}
	
//		amTrace((const U8*)"%x\t",(unsigned int)data);
//    }

//}

 if(bTimeout==TRUE) amTrace((const U8*)"Timeout on ch: %d\t",channel);

}
/* gets info about connected devices via MIDI interface */
const S8 *getConnectedDeviceInfo(void){
  U8 channel;
  
  /*  request on all channels */
  amTrace((const U8*)"Quering connected MIDI device...\n");
  
  for(channel=0;channel<0x7f;channel++){
    getDeviceInfoResponse(channel);
   }

 return NULL;
}

const U8 *am_getMidiDeviceTypeName(eMidiDeviceType device){
 if(device>=0&&device<DT_NUM_DEVICES)  return g_arMidiDeviceTypeName[device];
 else return NULL;
}

#ifdef DEBUG_BUILD
/* variable quantity reading test */
void VLQtest(void){
/* VLQ test */
    U32 val[]={0x00, 0x7F,0x8100,0xC000,0xFF7F,0x818000, 0xFFFF7F,0x81808000,0xC0808000,0xFFFFFF7F };
    U32 iCounter;
    
    U8 valsize;
    
    amTrace((const U8*)"VLQ decoding test\n");
    
    for (iCounter=0;iCounter<10;iCounter++)   {
        U8 *pValPtr=NULL;
		valsize=0;
		U32 result=0;
 		
		pValPtr=(U8 *)(&val[iCounter]);
        
		while((*pValPtr)==0x00){pValPtr++;}
        
		result = readVLQ(pValPtr,&valsize);
     	amTrace((const U8*)"VLQ value:%x, decoded: %x, size: %d\n",(unsigned int)val[iCounter], (unsigned int)result, valsize );
    }
    /* End of VLQ test */
}
#endif








