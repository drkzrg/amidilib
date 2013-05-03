
/**  Copyright 2007-2010 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#include "amidilib.h"

#ifndef PORTABLE
#include <mint/ostruct.h>
#include "timing/mfp.h"
#endif

#include "fmio.h"
#include "xmidi.h"
#include "roland.h"
#include "timing/miditim.h"

#include "list/list.h"
#include "config.h"

#ifdef TIME_CHECK_PORTABLE
#include <time.h>
#endif


static const sAMIDI_version version = { AMIDI_MAJOR_VERSION, AMIDI_MINOR_VERSION, AMIDI_PATCHLEVEL };

/* for saving last running status */
static U8 g_runningStatus;

#ifdef DEBUG_BUILD
static const U8 outputFilename[] = "amidi.log";
#endif

//default configuration filename
static const U8 configFilename[] = "amidi.cfg";

#ifdef TIME_CHECK_PORTABLE	
 clock_t begin;
 clock_t end;
#else
 unsigned long begin;
 unsigned long end;
 long usp;
#endif

/* */ 
static const U8 *g_arMidiDeviceTypeName[]={
  "Roland MT-32",       
  "Roland CM-32L",   
  "GS/GM",       
  "LA/GS mixed mode",
  "MT-32 GM emulation",
  "Yamaha XG GM mode"
};
 

/* static table with MIDI controller names */
extern const U8 *g_arMIDIcontrollers[];

/* static table with MIDI notes to ASA ISO */
extern const char *g_arMIDI2key[];

/* static table with CM32 rhytm part */
extern const char *g_arCM32Lrhythm[];


const sAMIDI_version *am_getVersionInfo(void){
  return (const sAMIDI_version *)(&version); 
}


S16 am_getHeaderInfo(void *pMidiPtr){
    sMThd *pMidiInfo=0;
    amTrace((const U8 *)"Checking header info... ");
    pMidiInfo=(sMThd *)pMidiPtr;
  
    /* check midi header */
    if(((pMidiInfo->id)==(ID_MTHD)&&(pMidiInfo->headLenght==6L))){

        switch(pMidiInfo->format){
	 case T_MIDI0:
	  /* Midi Format 0 detected */
	  amTrace((const U8*)"MIDI type 0 found\n");
	  return (T_MIDI0);
	 break;

	case T_MIDI1:
	 /* Midi Format 1 detected */
	 amTrace((const U8*)"MIDI type 1 found\n");
     
	 return(T_MIDI1);
	 break;

        case T_MIDI2:
	/* Midi Format 2 detected */
	amTrace((const U8*)"MIDI type 2 found\n");
	  return(T_MIDI2);
	break;

	default:
	/*Error: Unsupported MIDI format */
	amTrace((const U8*)"Unsupported MIDI file format\n");
    
	return(-2);
	break;
    }

   }
   else if ((pMidiInfo->id==ID_FORM)||(pMidiInfo->id==ID_CAT)){
      /* possible XMIDI*/
      amTrace((const U8*)"XMIDI file possibly..\n");
    
    return(3);
   }
    
return(-1);
}

S16 am_handleMIDIfile(void *pMidiPtr, U32 lenght, sSequence_t **pSequence){
    S16 iNumTracks=0;
    S16 iError=0;
    U16 iTimeDivision=0;
    void *startPtr=pMidiPtr;
    void *endPtr=0L;
    
    (*pSequence)=0;
    (*pSequence)=(sSequence_t *)amMallocEx(sizeof(sSequence_t),PREFER_TT);
    
    if((*pSequence)==0){ 
      return -1;
    }
   
   amMemSet((*pSequence),0,sizeof(sSequence_t));
   (*pSequence)->eotThreshold=EOT_SILENCE_THRESHOLD;
   (*pSequence)->ubActiveTrack=0;
    
   int iRet=0;
   iRet=am_getHeaderInfo(pMidiPtr);
    
   if(iRet==-1){
    /* not MIDI file, do nothing */
    amTrace((const U8*)"It's not valid MIDI file...\n");
    fprintf(stderr, "It's not valid MIDI file...\n");
    return -1;
   } else if(iRet==-2){
    /* unsupported MIDI type format, do nothing*/
    amTrace((const U8*)"Unsupported MIDI file format...\n");
    fprintf(stderr, "Unsupported MIDI file format...\n");
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
                 iTimeDivision = am_getTimeDivision(pMidiPtr);

		 /* process track data, offset the start pointer a little to get directly to track data and decode MIDI events */
                 startPtr=(void *)((U32)startPtr+sizeof(sMThd));

		 /* create one track list only */
		  (*pSequence)->arTracks[0] = (sTrack_t *)amMallocEx(sizeof(sTrack_t),PREFER_TT);
		  amMemSet((*pSequence)->arTracks[0],0,sizeof(sTrack_t));
		  /* Store time division for sequence, TODO: SMPTE handling */
		  
		  (*pSequence)->arTracks[0]->currentState.currentPPQN=am_decodeTimeDivisionInfo(iTimeDivision);	/* PPQN */
		 
		    
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
          startPtr=(void *)((U32)startPtr+sizeof(sMThd));
                	
	  /* Store time division for sequence, TODO: SMPTE handling */
	  (*pSequence)->ubNumTracks=iNumTracks;
	  
	  /* create one track list only */
	  for(int i=0;i<iNumTracks;i++){
	  (*pSequence)->arTracks[i] = (sTrack_t *)amMallocEx(sizeof(sTrack_t),PREFER_TT);
	  amMemSet((*pSequence)->arTracks[i],0,sizeof(sTrack_t));
	   /* init event list */
	   (*pSequence)->arTracks[i]->currentState.currentPPQN=am_decodeTimeDivisionInfo(iTimeDivision);	/* PPQN */
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
	  startPtr=(void *)((U32)startPtr+sizeof(sMThd));
		
	  /* Store time division for sequence, TODO: SMPTE handling */
	  (*pSequence)->ubNumTracks=iNumTracks;
	  
	  /* create one track list only */
	  for(int i=0;i<iNumTracks;i++){
	    (*pSequence)->arTracks[i] = (sTrack_t *)amMallocEx(sizeof(sTrack_t),PREFER_TT);
	    amMemSet((*pSequence)->arTracks[i],0,sizeof(sTrack_t));
	    /* init event list */
	    (*pSequence)->arTracks[i]->pTrkEventList=0;
	    (*pSequence)->arTracks[i]->currentState.currentPPQN=am_decodeTimeDivisionInfo(iTimeDivision);	/* PPQN */
	  
	  }
           
           while (startPtr!=0){
             startPtr=processMidiTrackData(startPtr,T_MIDI2,iNumTracks,pSequence,&iError);
	     if(iError<0)return iError;
           }
           
             return(0);
            }
        break;
	case T_XMIDI:{
         /* handle XMIDI */
         iNumTracks=am_getNbOfTracks(pMidiPtr,T_XMIDI);
         iTimeDivision = am_getTimeDivision(pMidiPtr);

         /* processing (X)MIDI file */
	/* TODO: handle + process */

         return(-1); /*xmidi isn't handled yet*/
        }
	break;
	case T_RMID:{return(-1);}break; 
	case T_SMF:{return(-1);}break;
	case T_XMF:{return(-1);}break;
	case T_SNG:{return(-1);}break;
	case T_MUS:{return(-1);}break;
	
	default:{
	  /* unknown error, do nothing */
	  amTrace((const U8*)"Unknown error.\n");
	  fprintf(stderr, "Unknown error ...\n");
        
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
	IFF_Chunk *pXmidiInfo=0;
	U32 ulNextChunk=0;
	U32 ulChunkOffset=0;
	U8 *Ptr=NULL;

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
     
     break;
     default:
       return -1;/*TODO: ! not implemented */
    }
return -1;
}

U16 am_getTimeDivision (void *pMidiPtr){
sMThd *pMidiInfo=(sMThd *)pMidiPtr;

/* check midi header */
if(((pMidiInfo->id)==(ID_MTHD)&&(pMidiInfo->headLenght==6L))){
  return (pMidiInfo->division);
}
    /* (X)Midi has timing data inside midi eventlist */

 return (0);
}

U8 am_calcRolandChecksum(U8 *buf_start, U8 *buf_end){
U8 total = 0 ;
U8 mask  = 0x7F ;

while ( buf_start <= buf_end ){
  total += *buf_start ;
  buf_start++ ;
}

 return (0x80 - (total & mask)) & mask ;
}


#ifndef PORTABLE
static U8 g_arMidiBuffer[MIDI_BUFFER_SIZE];

/* Midi buffers system info */
static _IOREC g_sOldMidiBufferInfo;
static _IOREC *g_psMidiBufferInfo;
#endif

extern BOOL CON_LOG;
extern FILE *ofp;

S16 am_init(){
    
#ifdef DEBUG_BUILD
 // init logger
 am_initLog(outputFilename);
#endif 
  
  loadConfig(configFilename);

  //save configuration
  if(saveConfig(configFilename)>=0L){
    printf("Configuration saved sucessfully.");
  }else{
    printf("Error: Cannot save global configuration.");
    return -1;
  }
  
#ifndef PORTABLE 
 /* clear our new buffer */
 U32 usp=0L;
 amMemSet(g_arMidiBuffer,0,MIDI_BUFFER_SIZE);

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
 (*g_psMidiBufferInfo).ibuflow=(U16)MIDI_LWM;
 (*g_psMidiBufferInfo).ibufhi=(U16)MIDI_HWM;
 SuperToUser(usp);
#endif

  
#ifndef PORTABLE 
#ifdef IKBD_MIDI_SEND_DIRECT
   MIDIbytesToSend=0;
#endif
#endif   
  
   
#ifdef EVENT_LINEAR_BUFFER

#endif   
   // now depending on the connected device type and chosen operation mode
   // set appropriate channel
   //prepare device for receiving messages
   
   switch(getGlobalConfig()->connectedDeviceType){
    case DT_LA_SOUND_SOURCE:     
    case DT_LA_SOUND_SOURCE_EXT:{
      amTrace("\nSetting MT32 device on ch: %d\n", getGlobalConfig()->midiChannel);
      program_change(getGlobalConfig()->midiChannel, 1);
    }break;
    
    case DT_GS_SOUND_SOURCE:       /* for pure GS/GM sound source */
    case DT_LA_GS_MIXED:           /* if both LA/GS sound sources are available, like in CM-500 */
    case DT_MT32_GM_EMULATION:     /* before loading midi data MT32 sound banks has to be patched */
    case DT_XG_GM_YAMAHA:
    default:{
      amTrace("\nSetting generic GM/GS device on ch: %d\n", getGlobalConfig()->midiChannel);
      control_change(C_BANK_SELECT, getGlobalConfig()->midiChannel,0,0x00);
      program_change(getGlobalConfig()->midiChannel, 1);
    }break;
    
   }

#ifndef PORTABLE 
#ifdef IKBD_MIDI_SEND_DIRECT
    amMidiSendIKBD();	
#endif
#endif   

#ifdef EVENT_LINEAR_BUFFER    
    if(initEventBuffer()<0){
       printf("Error: Couldn't allocate memory for internal midi event buffer.\n");
    }
#endif

   //TODO: interrogate connected external module type
   //check external module communication scheme
 if(getGlobalConfig()->handshakeModeEnabled){
    
    //display info 
     //if timeout turn off handshake mode
    //   for (U8 i=0;i<16;++i){
    // getDeviceInfoResponse(i);
    //    }
    ;
 }
   
   
 return 1;
}

void am_deinit(){

#ifdef EVENT_LINEAR_BUFFER
    destroyEventBuffer();
#endif  
  
#ifndef PORTABLE
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
  am_deinitLog();
#endif  
 /* end sequence */
}

void am_dumpMidiBuffer(){
#ifndef PORTABLE
  
  if(g_arMidiBuffer[0]!=0){
  amTrace((const U8*)"MIDI buffer dump:\n %s",g_arMidiBuffer);
 }
#endif
}


/* at this point pCurSequence should have the info about the type of file that resides in memory,
because we have to know if we have to dump event data to one eventlist or several ones */

/* all the events found in the track will be dumped to the sSequenceState_t structure  */


void *processMidiTrackData(void *startPtr, U32 fileTypeFlag,U32 numTracks, sSequence_t **pCurSequence, S16 *iError )
{   
U32 trackCounter=0;
U32 endAddr=0L;
U32 ulChunkSize=0;

sChunkHeader *pHeader=0;
sTrack_t **ppTrack=0;
void *end=0;

amTrace((const U8*)"Number of tracks to process: %d\n\n",numTracks);

pHeader=(sChunkHeader *)startPtr;
startPtr=(U8*)startPtr + sizeof(sChunkHeader);
ulChunkSize=pHeader->headLenght;
endAddr=(U32)startPtr+ulChunkSize;

switch(fileTypeFlag){
  
    case T_MIDI0:{
	  /* we have only one track data to process */
	  /* add all of them to given track */ 
	  sTrack_t *pTempTrack=(*pCurSequence)->arTracks[0];

	  pTempTrack->currentState.playMode = getGlobalConfig()->playMode;
	  pTempTrack->currentState.playState = getGlobalConfig()->playState;

	  pTempTrack->currentState.currentPPQN=DEFAULT_PPQN;
	  pTempTrack->currentState.currentTempo=DEFAULT_MPQN;
	  pTempTrack->currentState.currentBPM=DEFAULT_BPM;

	  ppTrack=&pTempTrack;
	  end=(void *)endAddr;
	  
	  startPtr=processMIDItrackEvents(&startPtr,(const void *)end,ppTrack, iError );
	  
	  if(iError<0) {
	    return NULL;
	  }
	  
	  //So coooooool......
	  pTempTrack->currentState.bMute=FALSE;
    }
    break;
     case T_MIDI1:{
      
      while(((pHeader!=0)&&(pHeader->id==ID_MTRK)&&(trackCounter<numTracks))){
	  /* we have got track data :)) */
	  /* add all of them to given track */ 
	  sTrack_t *pTempTrack=(*pCurSequence)->arTracks[trackCounter];
	  
	  pTempTrack->currentState.playMode = getGlobalConfig()->playMode;;
	  pTempTrack->currentState.playState = getGlobalConfig()->playState;
	  pTempTrack->currentState.currentPPQN=DEFAULT_PPQN;
	  pTempTrack->currentState.currentTempo=DEFAULT_MPQN;
	  pTempTrack->currentState.currentBPM=DEFAULT_BPM;
	  
	  ppTrack=&pTempTrack;
	  end=(void *)endAddr;
	  
	  startPtr=processMIDItrackEvents(&startPtr,(const void *)end,ppTrack, iError );
	
	  if(*iError<0) {
	    return NULL;
	  }
	  
	  //So coooooool......
	  pTempTrack->currentState.bMute=FALSE;
	  
	  /* increase track counter */
	  ++trackCounter;
	  
	  //prevent reading chunk after processing the last track
	  if(trackCounter<numTracks){
	    /* get next data chunk info */
	    pHeader=(sChunkHeader *)startPtr;
	    ulChunkSize=pHeader->headLenght;
	
	    /* omit Track header */
	    startPtr=(U8*)startPtr+sizeof(sChunkHeader);
	    endAddr=(U32)startPtr+ulChunkSize;
	  }else{
	    //just to play safe
	    pHeader=0;
	    ulChunkSize=0;
	    startPtr=0;
	    endAddr=0;
	  }
	}
    }break;
    case T_MIDI2:{
	/* handle MIDI 2, multitrack type */
	/* create several track lists according to numTracks */

	/* tracks inited, now insert track data */
	while(((pHeader!=0)&&(pHeader->id==ID_MTRK)&&(trackCounter<numTracks))){
	  /* we have got track data :)) */
	  /* add all of them to given track */ 
	  sTrack_t *pTempTrack=(*pCurSequence)->arTracks[trackCounter];

	  pTempTrack->currentState.playMode = getGlobalConfig()->playMode;;
	  pTempTrack->currentState.playState = getGlobalConfig()->playState;
	  pTempTrack->currentState.currentPPQN=DEFAULT_PPQN;
	  pTempTrack->currentState.currentTempo=DEFAULT_MPQN;
	  pTempTrack->currentState.currentBPM=DEFAULT_BPM;
	  
	  ppTrack=&pTempTrack;
	  end=(void *)endAddr;
	  
	  startPtr=processMIDItrackEvents(&startPtr,(const void *)end,ppTrack, iError );
	  
	  if(*iError<0) {
	    return NULL;
	  }
	  
	  //So coooooool......
	  pTempTrack->currentState.bMute=FALSE;
	  
	  /* increase track counter */
	  ++trackCounter;
	  
	  if(trackCounter<numTracks){
	    /* get next data chunk info */
	    pHeader=(sChunkHeader *)startPtr;
	    ulChunkSize=pHeader->headLenght;
	
	    /* omit Track header */
	    startPtr=(U8*)startPtr+sizeof(sChunkHeader);
	    endAddr=(U32)startPtr+ulChunkSize;
	  }else{
	    //just to play safe
	    pHeader=0;
	    ulChunkSize=0;
	    startPtr=0;
	    endAddr=0;
	  }
	}
    }break;
     case T_XMIDI:{
        /*TODO: ! not implemented */
	return NULL;
     }
     case T_RMID:{
     return NULL;/*TODO: ! not implemented */
     }break;
     case T_SMF:{
      return NULL;/*TODO: ! not implemented */
     }break;
     case T_XMF:{
      return NULL;/*TODO: ! not implemented */
     } break;
     
     case T_SNG:{
      return NULL;;/*TODO: ! not implemented */
    }break;
     case T_MUS:
       return NULL;;/*TODO: ! not implemented */
     break;
    default:{
      return NULL;
    }
  };

 amTrace((const U8*)"Finished processing...\n");
 return 0;
}

U8 am_isMidiChannelEvent(U8 byteEvent){

    if(( ((byteEvent&0xF0)>=0x80) && ((byteEvent&0xF0)<=0xE0)))
    {return 1;}
    else return 0;
}

U8 am_isMidiRTorSysex(U8 byteEvent){

    if( ((byteEvent>=(U8)0xF0)&&(byteEvent<=(U8)0xFF)) ){   
      /* it is! */
        return (1);
    }
    else /*no, it's not! */
        return (0);
}

/* handles the events in tracks and returns pointer to the next midi track */
void *processMIDItrackEvents(void**startPtr, const void *endAddr, sTrack_t **pCurTrack, S16 *iError ){
U8 usSwitch=0;
U16 recallStatus=0;
U32 delta=0L;
BOOL bEOF=FALSE;

    /* execute as long we are on the end of file or EOT meta occured, 
      50% midi track headers is broken, so the web says ;)) */
    
    U8 *pCmd=((U8 *)(*startPtr));
    U8 ubSize=0;

    while ( ((pCmd!=endAddr)&&(bEOF!=TRUE)&&(*iError>=0)) ){
    
      /*read delta time, pCmd should point to the command data */
      delta=readVLQ(pCmd,&ubSize);
     
      pCmd=(U8 *)((U32)pCmd+ubSize*sizeof(U8));

      /* handling of running status */
      /* if byte is not from 0x08-0x0E range then recall last running status AND set recallStatus = 1 */
      /* else set recallStatus = 0 and do nothing special */
      ubSize=(*pCmd);
      if( (!(am_isMidiChannelEvent(ubSize))&&(recallStatus==1)&&(!(am_isMidiRTorSysex(ubSize))))){
	/*recall last cmd byte */
	usSwitch=g_runningStatus;
        usSwitch=((usSwitch>>4)&0x0F);
      }else{
	/* check if the new cmd is the system one*/
	recallStatus=0;

      if((am_isMidiRTorSysex(ubSize))){
	usSwitch=ubSize;
      }else{
	usSwitch=ubSize;
        usSwitch=((usSwitch>>4)&0x0F);
      }
   }

    /* decode event and write it to our custom structure */
    switch(usSwitch){
      case EV_NOTE_OFF:
		*iError=am_noteOff(&pCmd,&recallStatus, delta, pCurTrack );
      break;
      case EV_NOTE_ON:
		*iError=am_noteOn(&pCmd,&recallStatus, delta, pCurTrack );
      break;
      case EV_NOTE_AFTERTOUCH:
		*iError=am_noteAft(&pCmd,&recallStatus, delta, pCurTrack );
      break;
      case EV_CONTROLLER:
		*iError=am_Controller(&pCmd,&recallStatus, delta, pCurTrack );
      break;
      case EV_PROGRAM_CHANGE:
		*iError=am_PC(&pCmd,&recallStatus, delta, pCurTrack );
      break;
      case EV_CHANNEL_AFTERTOUCH:
		*iError=am_ChannelAft(&pCmd,&recallStatus, delta, pCurTrack );
      break;
      case EV_PITCH_BEND:
		*iError=am_PitchBend(&pCmd,&recallStatus, delta, pCurTrack );
      break;
      case EV_META:
		*iError=am_Meta(&pCmd, delta, pCurTrack,&bEOF);
      break;
      case EV_SOX:                          	/* SySEX midi exclusive */
		recallStatus=0; 	                /* cancel out midi running status */
		*iError=(S16)am_Sysex(&pCmd,delta, pCurTrack);
      break;
      case SC_MTCQF:
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
      case SC_UNDEF2:                  /* undefined */
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
    }
} /*end of decode events loop */

    
  /* return the next track data */
  return(pCmd);
 
}


S16 am_noteOff(U8 **pPtr,U16 *recallRS,U32 delta, sTrack_t **pCurTrack){
sEventBlock_t tempEvent; 			
sNoteOff_EventBlock_t *pEvntBlock=NULL;
sNoteOff_t *pNoteOff=0;
tempEvent.dataPtr=0;

#ifdef MIDI_PARSER_DEBUG	
assert(sizeof(sNoteOff_t)==2);
#endif

if((*recallRS)==0){
  /* save last running status */
  g_runningStatus=*(*pPtr);
	
  tempEvent.uiDeltaTime=delta;
  tempEvent.type=T_NOTEOFF;
  getEventFuncInfo(T_NOTEOFF,&tempEvent.sendEventCb);
  getEventFuncCopyInfo(T_NOTEOFF,&tempEvent.copyEventCb);
  
  tempEvent.dataPtr=alloca(tempEvent.sendEventCb.size);
  
  pEvntBlock=(sNoteOff_EventBlock_t *)tempEvent.dataPtr;
  pEvntBlock->ubChannelNb=g_runningStatus&0x0F;

  /* now we can recall former running status next time */
  (*recallRS)=1;

  (*pPtr)++;
  pNoteOff=(sNoteOff_t *)(*pPtr);
  
  /* get parameters */
  pEvntBlock->eventData.noteNb=pNoteOff->noteNb;
  pEvntBlock->eventData.velocity=pNoteOff->velocity;
			
  (*pPtr)=(*pPtr)+sizeof(sNoteOff_t);

}else {
  /* recall last cmd status */
  /* and get parameters as usual */

  /* get last note info */
  tempEvent.uiDeltaTime=delta;
  tempEvent.type=T_NOTEOFF;
  getEventFuncInfo(T_NOTEOFF,&tempEvent.sendEventCb);
  getEventFuncCopyInfo(T_NOTEOFF,&tempEvent.copyEventCb);
  
  tempEvent.dataPtr=alloca(tempEvent.sendEventCb.size);

  pEvntBlock=(sNoteOff_EventBlock_t *)tempEvent.dataPtr;
  
  /* save channel */
  pEvntBlock->ubChannelNb=g_runningStatus&0x0F;

  pNoteOff=(sNoteOff_t *)(*pPtr);

  /* get parameters */
  pEvntBlock->eventData.noteNb=pNoteOff->noteNb;
  pEvntBlock->eventData.velocity=pNoteOff->velocity;
			
  (*pPtr)=(*pPtr)+sizeof(sNoteOff_t);
}

#ifdef MIDI_PARSER_DEBUG	
  amTrace((const U8*)"delta: %u\t",(unsigned long)delta);
  amTrace((const U8*)"event: Note off ");
  amTrace((const U8*)"ch: %d\t",(pEvntBlock->ubChannelNb)+1);
  amTrace((const U8*)"note: %d(%s)\t",pNoteOff->noteNb,am_getMIDInoteName(pNoteOff->noteNb));
  amTrace((const U8*)"vel: %d\n",pNoteOff->velocity);
#endif

  /* add event to list */
  return addEvent(&(*pCurTrack)->pTrkEventList, &tempEvent );
}

//
S16 am_noteOn(U8 **pPtr,U16 *recallRS,U32 delta, sTrack_t **pCurTrack){
sEventBlock_t tempEvent;

 U8 channel=0;
 U8 note=0;
 U8 velocity=0;
 sNoteOn_EventBlock_t *pEvntBlock=NULL;
 tempEvent.dataPtr=0;

 if((*recallRS)==0){
    /* save last running status */
  g_runningStatus=*(*pPtr);

  tempEvent.uiDeltaTime=delta;
  tempEvent.type=T_NOTEON;
  getEventFuncInfo(T_NOTEON,&tempEvent.sendEventCb);
  getEventFuncCopyInfo(T_NOTEON,&tempEvent.copyEventCb);
  
  tempEvent.dataPtr=alloca(tempEvent.sendEventCb.size);
  
  /*assert(tempEvent.dataPtr>0);*/
  pEvntBlock=(sNoteOn_EventBlock_t *)tempEvent.dataPtr;

  /* now we can recall former running status next time */
  (*recallRS)=1;

  (*pPtr)++;
  channel=(g_runningStatus&0x0F)+1;
  pEvntBlock->ubChannelNb=g_runningStatus&0x0F;
  note=*(*pPtr);
  pEvntBlock->eventData.noteNb=*(*pPtr);

  /* get parameters */
  (*pPtr)++;
  velocity=*(*pPtr);
  pEvntBlock->eventData.velocity=*(*pPtr);
    
  (*pPtr)++;
 }else{
    /* get last note info */
	channel=(g_runningStatus&0x0F)+1;

	tempEvent.uiDeltaTime=delta;
	tempEvent.type=T_NOTEON;
	getEventFuncInfo(T_NOTEON,&tempEvent.sendEventCb);
	getEventFuncCopyInfo(T_NOTEON,&tempEvent.copyEventCb);
	tempEvent.dataPtr=alloca(tempEvent.sendEventCb.size);

	pEvntBlock=(sNoteOn_EventBlock_t *)tempEvent.dataPtr;

	pEvntBlock->ubChannelNb=g_runningStatus&0x0F;

	note=*(*pPtr);
	pEvntBlock->eventData.noteNb=*(*pPtr);
	/* get parameters */
	(*pPtr)++;
	velocity=*(*pPtr);
	pEvntBlock->eventData.velocity=*(*pPtr);
	
	(*pPtr)++;
 }

 #ifdef MIDI_PARSER_DEBUG
    amTrace((const U8*)"delta: %u\t",(unsigned long)delta);
    amTrace((const U8*)"event: Note on ");
    amTrace((const U8*)"ch: %d\t",channel);
    amTrace((const U8*)"note: %d(%s)\t",note,am_getMIDInoteName(note));
    amTrace((const U8*)"vel: %d \n",velocity);
 #endif 
    
    /* add event to list */
    return addEvent(&(*pCurTrack)->pTrkEventList, &tempEvent );

}

S16 am_noteAft(U8 **pPtr,U16 *recallRS,U32 delta, sTrack_t **pCurTrack){
sEventBlock_t tempEvent;
U8 noteNb=0;
U8 pressure=0;
sNoteAft_EventBlock_t *pEvntBlock=NULL;

 if((*recallRS)==0){
    /* save last running status */
    g_runningStatus=*(*pPtr);
	
    tempEvent.uiDeltaTime=delta;
    tempEvent.type=T_NOTEAFT;
    getEventFuncInfo(T_NOTEAFT,&tempEvent.sendEventCb);
    getEventFuncCopyInfo(T_NOTEAFT,&tempEvent.copyEventCb);
    tempEvent.dataPtr=alloca(tempEvent.sendEventCb.size);
    pEvntBlock=(sNoteAft_EventBlock_t *)tempEvent.dataPtr;
    pEvntBlock->ubChannelNb=g_runningStatus&0x0F;

    /* now we can recall former running status next time */
    (*recallRS)=1;

    (*pPtr)++;
    /* get parameters */
    noteNb=*(*pPtr);
    pEvntBlock->eventData.noteNb=*(*pPtr);
    (*pPtr)++;
    pressure=*(*pPtr);
    pEvntBlock->eventData.pressure=*(*pPtr);
    (*pPtr)++;
	
 }else{
        /* get parameters */
	tempEvent.uiDeltaTime=delta;
	tempEvent.type=T_NOTEAFT;
	getEventFuncInfo(T_NOTEAFT,&tempEvent.sendEventCb);
	getEventFuncCopyInfo(T_NOTEAFT,&tempEvent.copyEventCb);

	tempEvent.dataPtr=alloca(tempEvent.sendEventCb.size);
	pEvntBlock=(sNoteAft_EventBlock_t *)tempEvent.dataPtr;
	pEvntBlock->ubChannelNb=g_runningStatus&0x0F;

        noteNb=*(*pPtr);
	pEvntBlock->eventData.noteNb=*(*pPtr);
        (*pPtr)++;
        pressure=*(*pPtr);
	pEvntBlock->eventData.pressure=*(*pPtr);
        (*pPtr)++;
    }
    
#ifdef MIDI_PARSER_DEBUG
     amTrace((const U8*)"delta: %u\tevent: Note Aftertouch note: %d, pressure: %d\n",(unsigned long)delta, noteNb,pressure);
#endif
    /* add event to list */
    return addEvent(&(*pCurTrack)->pTrkEventList, &tempEvent );
}

S16 am_Controller(U8 **pPtr,U16 *recallRS,U32 delta, sTrack_t **pCurTrack){
sEventBlock_t tempEvent;

  U8 channelNb=0;
    U8 controllerNb=0;
    U8 value=0;
    sController_EventBlock_t *pEvntBlock=NULL;
    tempEvent.dataPtr=0;

    if((*recallRS)==0){
        /* save last running status */
        g_runningStatus=*(*pPtr);
        /* now we can recall former running status next time */
        (*recallRS)=1;

	tempEvent.uiDeltaTime=delta;
	tempEvent.type=T_CONTROL;
	getEventFuncInfo(T_CONTROL,&tempEvent.sendEventCb);
	getEventFuncCopyInfo(T_CONTROL,&tempEvent.copyEventCb);
	
	tempEvent.dataPtr=alloca(tempEvent.sendEventCb.size);
	pEvntBlock=(sController_EventBlock_t *)tempEvent.dataPtr;

        channelNb=g_runningStatus&0x0F;
	pEvntBlock->ubChannelNb=g_runningStatus&0x0F;
        (*pPtr)++;
        /* get controller nb */
        controllerNb=(*(*pPtr));
	pEvntBlock->eventData.controllerNb=(*(*pPtr));
        (*pPtr)++;
        value=*((*pPtr));
	pEvntBlock->eventData.value=*((*pPtr));
        (*pPtr)++;
    }else{
        channelNb=g_runningStatus&0x0F;
	tempEvent.uiDeltaTime=delta;
	tempEvent.type=T_CONTROL;
	getEventFuncInfo(T_CONTROL,&tempEvent.sendEventCb);
	getEventFuncCopyInfo(T_CONTROL,&tempEvent.copyEventCb);
	
	tempEvent.dataPtr=alloca(tempEvent.sendEventCb.size);
	/*assert(tempEvent.dataPtr>0);*/
	pEvntBlock=(sController_EventBlock_t *)tempEvent.dataPtr;
	pEvntBlock->ubChannelNb=g_runningStatus&0x0F;

	/* get program controller nb */
        controllerNb=(*(*pPtr));
	pEvntBlock->eventData.controllerNb=(*(*pPtr));
        (*pPtr)++;
        value=*((*pPtr));
	pEvntBlock->eventData.value=*((*pPtr));
        (*pPtr)++;
    }
#ifdef MIDI_PARSER_DEBUG
    amTrace((const U8*)"delta: %u\tevent: Controller ch: %d, nb:%d name: %s\tvalue: %d\n",(unsigned long)delta, channelNb+1, controllerNb,getMIDIcontrollerName(controllerNb), value);
#endif 

    /* add event to list */
    return addEvent(&(*pCurTrack)->pTrkEventList, &tempEvent);
}

S16 am_PC(U8 **pPtr,U16 *recallRS,U32 delta, sTrack_t **pCurTrack){
sEventBlock_t tempEvent;

  U8 channel=0;
	U8 PN=0;
	sPrgChng_EventBlock_t *pEvntBlock=NULL;
     tempEvent.dataPtr=0;

     if((*recallRS)==0){
        /* save last running status */
        g_runningStatus=*(*pPtr);
		
	/* now we can recall former running status next time */
        (*recallRS)=1;

	channel=(g_runningStatus&0x0F)+1;

	tempEvent.uiDeltaTime=delta;
	tempEvent.type=T_PRG_CH;
	getEventFuncInfo(T_PRG_CH,&tempEvent.sendEventCb);
	getEventFuncCopyInfo(T_PRG_CH,&tempEvent.copyEventCb);
	
	tempEvent.dataPtr=alloca(tempEvent.sendEventCb.size);

	pEvntBlock=(sPrgChng_EventBlock_t *)tempEvent.dataPtr;
	pEvntBlock->ubChannelNb=g_runningStatus&0x0F;
        (*pPtr)++;
        /* get parameters */
        PN=*(*pPtr);
	pEvntBlock->eventData.programNb=*(*pPtr);
	(*pPtr)++;
    }else{
         /* get last PC status */
          channel=(g_runningStatus&0x0F)+1;
	  tempEvent.uiDeltaTime=delta;
	  tempEvent.type=T_PRG_CH;
	  getEventFuncInfo(T_PRG_CH,&tempEvent.sendEventCb);
	  getEventFuncCopyInfo(T_PRG_CH,&tempEvent.copyEventCb);
	  
	  tempEvent.dataPtr=alloca(tempEvent.sendEventCb.size);

	  pEvntBlock=(sPrgChng_EventBlock_t *)tempEvent.dataPtr;
	  pEvntBlock->ubChannelNb=g_runningStatus&0x0F;

          PN=*(*pPtr);
	  pEvntBlock->eventData.programNb=*(*pPtr);
         
	 /* get parameters */
	  (*pPtr)++;
    }
#ifdef MIDI_PARSER_DEBUG
    amTrace((const U8*)"delta: %u\t",(unsigned long)delta);
    amTrace((const U8*)"event: Program change ");
    amTrace((const U8*)"ch: %d\t",channel);
    amTrace((const U8*)"program nb: %d\n",PN);
#endif
    
  /* add event to list */
  return addEvent(&(*pCurTrack)->pTrkEventList, &tempEvent );
 }

S16 am_ChannelAft(U8 **pPtr,U16 *recallRS,U32 delta, sTrack_t **pCurTrack)
{
sEventBlock_t tempEvent;

  U8 channel=0;
U8 param=0;
sChannelAft_EventBlock_t *pEvntBlock=NULL;
tempEvent.dataPtr=0;

if((*recallRS)==0){
        /* save last running status */
        g_runningStatus=*(*pPtr);
        /* now we can recall former running status next time */
        (*recallRS)=1;

	tempEvent.uiDeltaTime=delta;
	tempEvent.type=T_CHAN_AFT;
	getEventFuncInfo(T_CHAN_AFT,&tempEvent.sendEventCb);
	getEventFuncCopyInfo(T_CHAN_AFT,&tempEvent.copyEventCb);
	
	tempEvent.dataPtr=alloca(tempEvent.sendEventCb.size);
	
	pEvntBlock=(sChannelAft_EventBlock_t *)tempEvent.dataPtr;
	pEvntBlock->ubChannelNb=(g_runningStatus&0x0F);
        
	/* get parameters */
        (*pPtr)++;
        param=*(*pPtr);
	pEvntBlock->eventData.pressure=*(*pPtr);
        (*pPtr)++;
    }else{
	tempEvent.uiDeltaTime=delta;
	tempEvent.type=T_CHAN_AFT;
	getEventFuncInfo(T_CHAN_AFT,&tempEvent.sendEventCb);
	getEventFuncCopyInfo(T_CHAN_AFT,&tempEvent.copyEventCb);
	
	tempEvent.dataPtr=alloca(tempEvent.sendEventCb.size);
	pEvntBlock=(sChannelAft_EventBlock_t *)tempEvent.dataPtr;
	pEvntBlock->ubChannelNb=(g_runningStatus&0x0F);

        /* get parameters */
	param=*(*pPtr);
	pEvntBlock->eventData.pressure=*(*pPtr);
        (*pPtr)++;
    }
#ifdef MIDI_PARSER_DEBUG
    amTrace((const U8*)"delta: %u\tevent: Channel aftertouch pressure: %d\n",(unsigned long)delta, param);
#endif
    /* add event to list */
    return addEvent(&(*pCurTrack)->pTrkEventList, &tempEvent );
}

S16 am_PitchBend(U8 **pPtr,U16 *recallRS,U32 delta, sTrack_t **pCurTrack){
sEventBlock_t tempEvent;

sPitchBend_EventBlock_t *pEvntBlock=NULL;
sPitchBend_t *pPitchBend=0;

#ifdef MIDI_PARSER_DEBUG
  assert(sizeof(sPitchBend_t)==2);
#endif

tempEvent.dataPtr=0;

    if((*recallRS)==0){
        /* save last running status */
        g_runningStatus=*(*pPtr);

        /* now we can recall former running status next time */
        (*recallRS)=1;

	tempEvent.uiDeltaTime=delta;
	tempEvent.type=T_PITCH_BEND;
	getEventFuncInfo(T_PITCH_BEND,&tempEvent.sendEventCb);
	getEventFuncCopyInfo(T_PITCH_BEND,&tempEvent.copyEventCb);
	
	tempEvent.dataPtr=alloca(tempEvent.sendEventCb.size);

	pEvntBlock=(sPitchBend_EventBlock_t *)tempEvent.dataPtr;
	pEvntBlock->ubChannelNb=(g_runningStatus&0x0F);

        (*pPtr)++;
	pPitchBend=(sPitchBend_t *)(*pPtr);
	
	/* get parameters */
	pEvntBlock->eventData.LSB=pPitchBend->LSB;
	pEvntBlock->eventData.MSB=pPitchBend->MSB;
        (*pPtr)=(*pPtr)+sizeof(sPitchBend_t);
    }else{
	tempEvent.uiDeltaTime=delta;
	tempEvent.type=T_PITCH_BEND;
	getEventFuncInfo(T_PITCH_BEND,&tempEvent.sendEventCb);
	getEventFuncCopyInfo(T_PITCH_BEND,&tempEvent.copyEventCb);
	
	tempEvent.dataPtr=alloca(tempEvent.sendEventCb.size);
	pEvntBlock=(sPitchBend_EventBlock_t *)tempEvent.dataPtr;
	pEvntBlock->ubChannelNb=(g_runningStatus&0x0F);

        /* get parameters */
        pPitchBend=(sPitchBend_t *)(*pPtr);
	
	/* get parameters */
	pEvntBlock->eventData.LSB=pPitchBend->LSB;
	pEvntBlock->eventData.MSB=pPitchBend->MSB;
        (*pPtr)=(*pPtr)+sizeof(sPitchBend_t);
    }
 #ifdef MIDI_PARSER_DEBUG
 amTrace((const U8*)"delta: %u\tevent: Pitch bend LSB: %d, MSB:%d\n",(unsigned long)delta,pPitchBend->LSB,pPitchBend->MSB);
 #endif

 /* add event to list */
  return addEvent(&(*pCurTrack)->pTrkEventList, &tempEvent );
 
}

S16 am_Sysex(U8 **pPtr,U32 delta, sTrack_t **pCurTrack){
  sEventBlock_t tempEvent;
  sSysEX_EventBlock_t *pEvntBlock=0;
  U8 *pTmpPtr=0;
  
  U32 ulCount=0L;
  tempEvent.dataPtr=0;

  tempEvent.uiDeltaTime=delta;
  tempEvent.type=T_SYSEX;
  getEventFuncInfo(T_SYSEX,&tempEvent.sendEventCb);
  getEventFuncCopyInfo(T_SYSEX,&tempEvent.copyEventCb);
  tempEvent.dataPtr=alloca(tempEvent.sendEventCb.size);
  
  pEvntBlock=(sSysEX_EventBlock_t *)tempEvent.dataPtr;
  
#ifdef MIDI_PARSER_DEBUG
  amTrace((const U8*)"SOX: ");
#endif  
   pTmpPtr=(*pPtr); //save start
   
   while( (*(*pPtr))!=EV_EOX){
#ifdef MIDI_PARSER_DEBUG
    amTrace((const U8*)"%x ",*(*pPtr));
#endif     
     (*pPtr)++;
      /*count Sysex msg data bytes */
      ulCount++;
    }
    pEvntBlock->bufferSize=ulCount; //size of data
    pEvntBlock->pBuffer=amMallocEx(ulCount*sizeof(U8),PREFER_TT);
    
    //copy ulCount of data
    amMemCpy(pEvntBlock->pBuffer,pTmpPtr,ulCount*sizeof(U8));
    
#ifdef MIDI_PARSER_DEBUG
    amTrace((const U8*)" EOX, size: %ld\n",ulCount);
#endif
    
 /* add event to list */
 return addEvent(&(*pCurTrack)->pTrkEventList, &tempEvent);
}

S16 am_Meta(U8 **pPtr,U32 delta, sTrack_t **pCurTrack, BOOL *bEOT){
sEventBlock_t tempEvent;

 U32 addr;
 U8 ubLenght,ubVal,ubSize=0;
 U8 textBuffer[64]={0};
 *bEOT=FALSE;
 
 tempEvent.dataPtr=0;
 
 /*get meta event type */
 (*pPtr)++;
 ubVal=*(*pPtr);

 switch(ubVal){
    case MT_SEQ_NB:{
#ifdef MIDI_PARSER_DEBUG
        amTrace((const U8*)"delta: %u\tMeta event: Sequence nb: ",(unsigned long)delta);
#endif
        (*pPtr)++;
        ubLenght=(*(*pPtr));
#ifdef MIDI_PARSER_DEBUG
        amTrace((const U8*)"%d\n", ubLenght);
#endif
        (*pPtr)++;
        addr=((U32)(*pPtr))+ubLenght*sizeof(U8);
        *pPtr=(U8*)addr;
	return 0;
    }break;
    case MT_TEXT:{
#ifdef MIDI_PARSER_DEBUG
        amTrace((const U8*)"delta: %u\tMeta event: Text:",(unsigned long)delta);
#endif
        (*pPtr)++;
        ubLenght=readVLQ((*pPtr),&ubSize);
#ifdef MIDI_PARSER_DEBUG
        amTrace((const U8*)"meta size: %d ",ubLenght);
#endif
	/* set to the start of the string */
        (*pPtr)++;
        amMemCpy(textBuffer, (*pPtr),ubLenght*sizeof(U8) );
        (*pPtr)=((*pPtr)+ubLenght);
#ifdef MIDI_PARSER_DEBUG
        amTrace((const U8*)"%s \n",textBuffer);
#endif
	return 0;
    }break;
    case MT_COPYRIGHT:{
#ifdef MIDI_PARSER_DEBUG
        amTrace((const U8*)"delta: %u\tMeta event: Copyright: ",(unsigned long)delta);
#endif
        (*pPtr)++;
        ubLenght=readVLQ((*pPtr),&ubSize);
#ifdef MIDI_PARSER_DEBUG
        amTrace((const U8*)"meta size: %d ",ubLenght);
#endif
        /* set to the start of the string */
        (*pPtr)++;
        amMemCpy(textBuffer, (*pPtr),ubLenght*sizeof(U8) );
        (*pPtr)=((*pPtr)+ubLenght);
#ifdef MIDI_PARSER_DEBUG
        amTrace((const U8*)"%s \n",textBuffer);
#endif
	return 0;
    }break;
    case MT_SEQNAME:{
#ifdef MIDI_PARSER_DEBUG
        amTrace((const U8*)"delta: %u\tMeta event: Sequence name: ",(unsigned long)delta);
#endif
        (*pPtr)++;
        ubLenght=readVLQ((*pPtr),&ubSize);
        /* set to the start of the string */
        (*pPtr)++;
        
	(*pCurTrack)->pTrackName=amMallocEx(128*sizeof(U8),PREFER_TT);
	
	if((*pCurTrack)->pTrackName!=NULL){
	  amMemSet((*pCurTrack)->pTrackName,0,128*sizeof(U8));
	  strncpy((char *)(*pCurTrack)->pTrackName,(char *)(*pPtr),ubLenght);
	  (*pCurTrack)->pTrackName[ubLenght]='\0';
	  //amMemCpy((*pCurTrack)->pTrackName, (*pPtr),ubLenght*sizeof(U8));
	}
	
        (*pPtr)=((*pPtr)+ubLenght);
#ifdef MIDI_PARSER_DEBUG
        amTrace((const U8*)"meta size: %d ",ubLenght);
        amTrace((const U8*)"%s \n",(*pCurTrack)->pTrackName);
#endif
	return 0;
    }break;
    case MT_INSTRNAME:{
#ifdef MIDI_PARSER_DEBUG
        amTrace((const U8*)"delta: %u\tMeta event: Instrument name: ",(unsigned long)delta);
#endif
        (*pPtr)++;
        ubLenght=readVLQ((*pPtr),&ubSize);
#ifdef MIDI_PARSER_DEBUG
        amTrace((const U8*)"meta size: %d",ubLenght);
#endif
        /* set to the start of the string */
        (*pPtr)++;
        amMemCpy(textBuffer, (*pPtr),ubLenght*sizeof(U8) );
        (*pPtr)=((*pPtr)+ubLenght);
	
#ifdef MIDI_PARSER_DEBUG
        amTrace((const U8*)"%s \n",textBuffer);
#endif
	return 0;
    }break;
    case MT_LYRICS:{
#ifdef MIDI_PARSER_DEBUG
        amTrace((const U8*)"delta: %u\tMeta event: Lyrics: ",(unsigned long)delta);
#endif
        (*pPtr)++;
        ubLenght=readVLQ((*pPtr),&ubSize);
#ifdef MIDI_PARSER_DEBUG
        amTrace((const U8*)"meta size: %d ",ubLenght);
#endif
        /* set to the start of the string */
        (*pPtr)++;
        amMemCpy(textBuffer, (*pPtr),ubLenght*sizeof(U8));
        (*pPtr)=((*pPtr)+ubLenght);
#ifdef MIDI_PARSER_DEBUG
        amTrace((const U8*)"%s \n",textBuffer);
#endif
	return 0;
    }break;

    case MT_MARKER:{
#ifdef MIDI_PARSER_DEBUG
        amTrace((const U8*)"delta: %u\tMeta event: Marker: ",(unsigned long)delta);
#endif
	sMarker_EventBlock_t *pEvntBlock=0;
	
        (*pPtr)++;
        ubLenght=readVLQ((*pPtr),&ubSize);
        
	/* set to the start of the string */
        (*pPtr)++;
        tempEvent.uiDeltaTime=delta;
	tempEvent.type=T_META_MARKER;
	getEventFuncInfo(T_META_MARKER,&tempEvent.sendEventCb);
	getEventFuncCopyInfo(T_META_MARKER,&tempEvent.copyEventCb);
	tempEvent.dataPtr=alloca(tempEvent.sendEventCb.size);
	
	pEvntBlock=(sMarker_EventBlock_t *)tempEvent.dataPtr;
	pEvntBlock->pMarkerName=amMallocEx(ubLenght+1,PREFER_TT);
	amMemSet(pEvntBlock->pMarkerName,0,((ubLenght+1)*sizeof(U8)));
	amMemCpy(pEvntBlock->pMarkerName,(*pPtr),ubLenght*sizeof(U8));
	pEvntBlock->pMarkerName[ubLenght]='\0';
	(*pPtr)=((*pPtr)+ubLenght);
	
	/* add event to list */
	addEvent(&(*pCurTrack)->pTrkEventList, &tempEvent );
	
#ifdef MIDI_PARSER_DEBUG
        amTrace((const U8*)"meta size: %d ",ubLenght);
        amTrace((const U8*)"%s \n",textBuffer);
#endif
	return 0;
    }
    break;
    case MT_CUEPOINT:{
#ifdef MIDI_PARSER_DEBUG
        amTrace((const U8*)"delta: %u\tMeta event: Cuepoint\n",(unsigned long)delta);
#endif
	sCuePoint_EventBlock_t *pEvntBlock=0;
	
        (*pPtr)++;
        ubLenght=readVLQ((*pPtr),&ubSize);
        /* set to the start of the string */
        (*pPtr)++;
        
	tempEvent.uiDeltaTime=delta;
	tempEvent.type=T_META_CUEPOINT;
	getEventFuncInfo(T_META_CUEPOINT,&tempEvent.sendEventCb);
	getEventFuncCopyInfo(T_META_CUEPOINT,&tempEvent.copyEventCb);
	
	tempEvent.dataPtr=alloca(tempEvent.sendEventCb.size);
	
	pEvntBlock=(sCuePoint_EventBlock_t *)tempEvent.dataPtr;
	pEvntBlock->pCuePointName=0;
	
	pEvntBlock->pCuePointName=amMallocEx(ubLenght+1,PREFER_TT);
	amMemSet(pEvntBlock->pCuePointName,0,((ubLenght+1)*sizeof(U8)));
	amMemCpy(pEvntBlock->pCuePointName,(*pPtr),ubLenght*sizeof(U8));
	pEvntBlock->pCuePointName[ubLenght]='\0';
	
	(*pPtr)=((*pPtr)+ubLenght);
	
	/* add event to list */
	addEvent(&(*pCurTrack)->pTrkEventList, &tempEvent );
	
#ifdef MIDI_PARSER_DEBUG
        amTrace((const U8*)"meta size: %d ",ubLenght);
        amTrace((const U8*)"%s \n",pEvntBlock->pCuePointName);
#endif
	return 0;
    }break;

    case MT_PROGRAM_NAME:{
        /* program(patch) name */
#ifdef MIDI_PARSER_DEBUG
        amTrace((const U8*)"delta: %u\tMeta event: Program (patch) name: ",(unsigned long)delta);
#endif
        (*pPtr)++;
        ubLenght=readVLQ((*pPtr),&ubSize);

        /* set to the start of the string */
        (*pPtr)++;
        amMemCpy(textBuffer, (*pPtr),ubLenght*sizeof(U8) );
        (*pPtr)=((*pPtr)+ubLenght);
	
#ifdef MIDI_PARSER_DEBUG
        amTrace((const U8*)"meta size: %d ",ubLenght);
        amTrace((const U8*)"%s \n",textBuffer);	
#endif
	return 0;
    }break;
    case MT_DEVICE_NAME:{
        /* device (port) name */
#ifdef MIDI_PARSER_DEBUG
	amTrace((const U8*)"delta: %u\tMeta event: Device (port) name: ",(unsigned long)delta);
#endif	
        (*pPtr)++;
        ubLenght=readVLQ((*pPtr),&ubSize);

        /* set to the start of the string */
        (*pPtr)++;
        amMemCpy(textBuffer, (*pPtr),ubLenght*sizeof(U8) );
        (*pPtr)=((*pPtr)+ubLenght);
#ifdef MIDI_PARSER_DEBUG
        amTrace((const U8*)"meta size: %d ",ubLenght);
        amTrace((const U8*)"%s \n",textBuffer);
#endif
	return 0;
    }break;
    case MT_CH_PREFIX:{
#ifdef MIDI_PARSER_DEBUG
        amTrace((const U8*)"delta: %u\tMeta event: Channel prefix\n",(unsigned long)delta);
#endif
        (*pPtr)++;
        ubLenght=(*(*pPtr));
        (*pPtr)++;
        addr=((U32)(*pPtr))+ubLenght*sizeof(U8);
        *pPtr=(U8*)addr;
	return 0;
    }break;
    case MT_MIDI_CH:{ /* obsolete! just ignore */
        (*pPtr)++;
        /*get size */
        ubLenght=(*(*pPtr));
        (*pPtr)++;

        /*get port nb*/
#ifdef MIDI_PARSER_DEBUG
        amTrace((const U8*)"delta: %u\tMeta event: Midi channel nb: %d\n",(unsigned long)delta,*(*pPtr));
#endif
        (*pPtr)++;
	return FALSE;
    }break;
    case MT_MIDI_PORT:{ /* obsolete! just ignore */
        (*pPtr)++;
        /*get size */
        ubLenght=(*(*pPtr));
        (*pPtr)++;

        /*get port nb*/
#ifdef MIDI_PARSER_DEBUG
        amTrace((const U8*)"delta: %u\tMeta event: Midi port nb: %d\n",(unsigned long)delta,*(*pPtr));
#endif
        (*pPtr)++;
	return 0;
    }break;
    case MT_EOT:{
#ifdef MIDI_PARSER_DEBUG
        amTrace((const U8*)"delta: %u\tMeta event: End of track\n\n",(unsigned long)delta);
#endif
	sEot_EventBlock_t *pEvntBlock=0;
        (*pPtr)++;
        ubLenght=(*(*pPtr));
        (*pPtr)++;
        addr=((U32)(*pPtr))+ubLenght*sizeof(U8);
        *pPtr=(U8*)addr;
	
	tempEvent.uiDeltaTime=delta;
	tempEvent.type=T_META_EOT;
	getEventFuncInfo(T_META_EOT,&tempEvent.sendEventCb);
	getEventFuncCopyInfo(T_META_EOT,&tempEvent.copyEventCb);
	
	tempEvent.dataPtr=alloca(tempEvent.sendEventCb.size);
	
	pEvntBlock=(sEot_EventBlock_t *)tempEvent.dataPtr;
	pEvntBlock->dummy=0L;		//dummy value
	
	*bEOT=TRUE;
	/* add event to list */
	return addEvent(&(*pCurTrack)->pTrkEventList, &tempEvent );
    }break;
    case MT_SET_TEMPO:{
        /* sets tempo in track, should be in the first track, if not 120 BPM is assumed */
	U32 val1,val2,val3;
	U8 ulVal[3]={0};   /* for retrieving set tempo info */
	sTempo_EventBlock_t *pEvntBlock=NULL;
	#ifdef MIDI_PARSER_DEBUG
	amTrace((const U8*)"delta: %u\tMeta event: Set tempo: ",(unsigned long)delta);
	#endif
	
	(*pPtr)++;
        ubLenght=(*(*pPtr));
         (*pPtr)++;
        /* get those 3 bytes */
        amMemCpy(ulVal, (*pPtr),ubLenght*sizeof(U8) );
        addr=((U32)(*pPtr))+ubLenght*sizeof(U8);
        *pPtr=(U8*)addr;
	
	val1=ulVal[0],val2=ulVal[1],val3=ulVal[2]; 
	val1=(val1<<16)&0x00FF0000L;
	val2=(val2<<8)&0x0000FF00L;
	val3=(val3)&0x000000FFL;

	/* range: 0-8355711 ms, 24 bit value */
	val1=val1|val2|val3;
	
	tempEvent.uiDeltaTime=delta;
	tempEvent.type=T_META_SET_TEMPO;
	getEventFuncInfo(T_META_SET_TEMPO,&tempEvent.sendEventCb);
	getEventFuncCopyInfo(T_META_SET_TEMPO,&tempEvent.copyEventCb);
	
	tempEvent.dataPtr=alloca(tempEvent.sendEventCb.size);
	
	pEvntBlock=(sTempo_EventBlock_t *)tempEvent.dataPtr;
	
        pEvntBlock->eventData.tempoVal=	val1;

#ifdef MIDI_PARSER_DEBUG
	amTrace((const U8*)"%u ms per quarter-note\n", (unsigned int)val1);
#endif
	/* add event to list */
	return addEvent(&(*pCurTrack)->pTrkEventList, &tempEvent );
    }
    break;
    case MT_SMPTE_OFFSET:{
       sSMPTEoffset SMPTEinfo;
#ifdef MIDI_PARSER_DEBUG
      amTrace((const U8*)"delta: %u\tMeta event: SMPTE offset:\n",(unsigned long)delta);
#endif	
        (*pPtr)++;
        ubLenght=(*(*pPtr));
        (*pPtr)++;
        amMemCpy (&SMPTEinfo,(*pPtr),sizeof(sSMPTEoffset));

        addr=((U32)(*pPtr))+ubLenght*sizeof(U8);
        *pPtr=(U8*)addr;
/* print out info */
#ifdef MIDI_PARSER_DEBUG
        amTrace((const U8*)"hr: %d\n",SMPTEinfo.hr);
        amTrace((const U8*)"mn: %d\n",SMPTEinfo.mn);
        amTrace((const U8*)"se: %d\n",SMPTEinfo.fr);
        amTrace((const U8*)"fr: %d\n",SMPTEinfo.fr);
        amTrace((const U8*)"ff: %d\n",SMPTEinfo.ff);
#endif
      return 0;
    }break;
    case MT_TIME_SIG:{
	 sTimeSignature timeSign;
	 sTimeSignature_EventBlock_t *pEvntBlock=NULL;
	 
#ifdef MIDI_PARSER_DEBUG
        amTrace((const U8*)"delta: %u\tMeta event: Time signature: ",(unsigned long)delta);
#endif
        (*pPtr)++;
        ubLenght=(*(*pPtr));
        (*pPtr)++;

        amMemCpy(&timeSign,(*pPtr),sizeof(sTimeSignature));
        addr=((U32)(*pPtr))+ubLenght*sizeof(U8);
        *pPtr=(U8*)addr;
	
	tempEvent.uiDeltaTime=delta;
	tempEvent.type = T_META_SET_SIGNATURE;
	getEventFuncInfo(T_META_SET_SIGNATURE,&tempEvent.sendEventCb);
	getEventFuncCopyInfo(T_META_SET_SIGNATURE,&tempEvent.copyEventCb);
	
	tempEvent.dataPtr=alloca(tempEvent.sendEventCb.size);
	pEvntBlock=(sTimeSignature_EventBlock_t *)tempEvent.dataPtr;
        pEvntBlock->timeSignature=timeSign;
	
        /* print out info */
#ifdef MIDI_PARSER_DEBUG
        amTrace((const U8*)"Time signature change nn: %d\tdd: %d\tcc: %d\tbb: %d\r\n",timeSign.nn,timeSign.dd,timeSign.cc,timeSign.bb);
#endif
	/* add event to list */
	return addEvent(&(*pCurTrack)->pTrkEventList, &tempEvent );
	
    }break;
    case MT_KEY_SIG:{
        U8 param1=0,param2=0;
 
#ifdef MIDI_PARSER_DEBUG
        amTrace((const U8*)"delta: %u\tMeta event: Key signature: ",(unsigned long)delta);
#endif
        (*pPtr)++;
        ubLenght=(*(*pPtr));
        (*pPtr)++;
        param1=(*(*pPtr));
        (*pPtr)++;
        param2=(*(*pPtr));
#ifdef MIDI_PARSER_DEBUG
        if (param2==0) {amTrace((const U8*)"scale: major ");}
        else if(param2==1) {amTrace((const U8*)"scale: minor ");}
         else { amTrace((const U8*)"error: wrong key signature scale. "); }

        if(param1==0)
            {amTrace((const U8*)"Key of C\n");}
        else if (((S8)param1==-1))
            {amTrace((const U8*)"1 flat\n");}
        else if(((S8)param1)==1)
            {amTrace((const U8*)"1 sharp\n");}
        else if ((param1>1&&param1<=7))
            {amTrace((const U8*)" %d sharps\n",param1);}
        else if (( ((S8)param1)<-1&& ((S8)param1)>=-7))
            {amTrace((const U8*)" %ld flats\n",(U32)param1);}
        else {amTrace((const U8*)" error: wrong key signature. %d\n",param1);}
#endif
		
        (*pPtr)++;
	return 0;
    }break;
    case MT_SEQ_SPEC:{
	#ifdef MIDI_PARSER_DEBUG
	  amTrace((const U8*)"delta: %u\tMeta event: Sequencer specific data.\n",(unsigned long)delta);
	#endif
        (*pPtr)++;
        ubLenght=(*(*pPtr));
        (*pPtr)++;
        addr=((U32)(*pPtr))+ubLenght*sizeof(U8);
        *pPtr=(U8*)addr;
	return 0;
    }break;
    default:{
	#ifdef MIDI_PARSER_DEBUG
       amTrace((const U8*)"delta: %u\tUnknown meta event.\n",(unsigned long)delta);
	#endif
        (*pPtr)++;
        ubLenght=(*(*pPtr));
        /* we should put here assertion failed or something with "send this file to author" message */
        /* file also could be broken */
	
	#ifdef MIDI_PARSER_DEBUG
        amTrace((const U8*)"id: %d, size: %d\n" /*parameters: %ld \n"*/,ubVal,*(*pPtr));
	#endif
        (*pPtr)=(*pPtr)+ubLenght;
	return 0;
    }break;
 }

}

/* reads Variable Lenght Quantity */
U32 readVLQ(U8 *pChar,U8 *ubSize){
// TODO: rewrite this in assembly, maybe make as inline 
U32 value=0;
U8 c=0;
(*ubSize)=0;
value = (*pChar);
    
if ( (value & 0x80) ){
       value &= 0x7F;

/* get next byte */
pChar++;
(*ubSize)++;

       do{
         value = (value << 7);
         c = (*pChar);
         value = value + (c&0x7F);

         pChar++;
          (*ubSize)++;
       } while (c & 0x80);
    }
    else{
     (*ubSize)++;
    }

return(value);
}

/* combine bytes function for pitch bend */
U16 combineBytes(U8 bFirst, U8 bSecond){
    U16 val;

    val = (U16)bSecond;
    val<<=7;
    val|=(U16)bFirst;
 return(val);
}

/* returns name of MIDI controller */
const U8 *getMIDIcontrollerName(U8 iNb)
{

 return(g_arMIDIcontrollers[iNb]);
}


void getDeviceInfoResponse(U8 channel){
  //TODO: rework it
  static U8 getInfoSysEx[]={0xF0,ID_ROLAND,GS_DEVICE_ID,GS_MODEL_ID,0x7E,0x7F,0x06,0x01,0x00,0xF7}; 
  //U8 getInfoSysEx[]={0xF0,0x41,0x10,0x42,0x7E,0x7F,0x06,0x01,0x00,0xF7};
  BOOL bFlag=FALSE;
  BOOL bTimeout=FALSE;
  U32 data=0;
  
  /* calculate checksum */
  getInfoSysEx[5]=am_calcRolandChecksum(&getInfoSysEx[2],&getInfoSysEx[4]);
  getInfoSysEx[5]=channel;
  getInfoSysEx[8]=am_calcRolandChecksum(&getInfoSysEx[5],&getInfoSysEx[7]);  

#ifdef IKBD_MIDI_SEND_DIRECT
  for(int i=0;i<10;i++){
    MIDIsendBuffer[MIDIbytesToSend++]=getInfoSysEx[i];
  }
  amMidiSendIKBD();
#else
  /* request data */
    MIDI_SEND_DATA(10,(void *)getInfoSysEx); 
   // am_dumpMidiBuffer(); 
#endif    
    begin=getTimeStamp(); // get current timestamp
	
    /* get reply or there was timeout */
    while((MIDI_DATA_READY&&(getTimeDelta()<getGlobalConfig()->midiConnectionTimeOut))) {
	data = GET_MIDI_DATA;
	begin=getTimeStamp(); // data received, reset timestamp
      
	 if(data!=0){
	  
	  if(bFlag==FALSE){
		amTrace((const U8*)"Received device info on ch: %d\t",channel);
 	    bFlag=TRUE;
	}
	
		amTrace((const U8*)"%x\t",(unsigned int)data);
    }
    
    
}

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
 
 am_dumpMidiBuffer();
 return NULL;
}

void am_allNotesOff(U16 numChannels){
U16 iCounter;
  for(iCounter=0;iCounter<numChannels;iCounter++){
  all_notes_off(iCounter);
 }
}        
#ifdef TIME_CHECK_PORTABLE	
/* utility for measuring function time execution in ms */
double am_diffclock(clock_t end,clock_t begin){
 double diffticks=end-begin;
 double diffms=(diffticks)/(CLOCKS_PER_SEC/1000.0f);
return diffms;
}
#endif

const U8 *am_getMIDInoteName(U8 ubNoteNb){
if((ubNoteNb>=0&&ubNoteNb<=127)) /* 0-127 range check */
 return((const U8*)g_arMIDI2key[ubNoteNb]);
else 
  return NULL;
}

const char *getNoteName(U8 currentChannel,U8 currentPN, U8 noteNumber){
  /* 0-127 range check */
  if((noteNumber>=0&&noteNumber<=127)) {  
    if((currentChannel==9&&currentPN==128)){
      return(g_arCM32Lrhythm[noteNumber]);
    }
    else{
	return(g_arMIDI2key[noteNumber]);
    }
}else 
  return NULL;
}

#ifdef DEBUG_BUILD
/* variable quantity reading test */
void VLQtest(void){
/* VLQ test */
    U32 val[]={0x00, 0x7F,0x8100,0xC000,0xFF7F,0x818000, 0xFFFF7F,0x81808000,0xC0808000,0xFFFFFF7F };
    U32 result=0,iCounter;
    U8 *pValPtr=NULL;
    U8 valsize;
    
    amTrace((const U8*)"VLQ decoding test\n");
    
    for (iCounter=0;iCounter<10;iCounter++)   {
        pValPtr=(U8 *)(&val[iCounter]);
        while((*pValPtr)==0x00)
        {pValPtr++;}
        valsize=0;result=0;
        result= readVLQ(pValPtr,&valsize);
     	amTrace((const U8*)"VLQ value:%x, decoded: %x, size: %d\n",(unsigned int)val[iCounter], (unsigned int)result, valsize );
	
    }
    /* End of VLQ test */
}
#endif

void am_destroySequence (sSequence_t **pPtr){
  #ifdef DEBUG_BUILD
    amTrace((const U8 *)"am_destroySequence() destroy sequence at %p initiated... 1..2..3... \n",*pPtr);
  #endif
    
  //go to the end of sequence
  if((*pPtr)->pSequenceName!=0){
      amFree((void **)&((*pPtr)->pSequenceName));
  }
  
  //destroy all tracks
   for (int i=0;i<AMIDI_MAX_TRACKS;i++){
     if((*pPtr)->arTracks[i]!=0){
      if((*pPtr)->arTracks[i]->pTrackName!=0) amFree((void **)&((*pPtr)->arTracks[i]->pTrackName));
      destroyList(&((*pPtr)->arTracks[i]->pTrkEventList));
      amFree((void **)&((*pPtr)->arTracks[i]));
     }
   }   
      
  //destroy sequence and nullify it
  amFree((void **)pPtr);      
  #ifdef DEBUG_BUILD
    amTrace((const U8 *)"am_destroySequence() done. \n");
  #endif
}


const U8 *am_getMidiDeviceTypeName(eMidiDeviceType device){
 if(device>=0&&device<DT_NUM_DEVICES)  return g_arMidiDeviceTypeName[device];
 else return NULL;
}


