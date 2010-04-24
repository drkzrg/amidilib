/**  Copyright 2008, 2009 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.

    AMIDILIB is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    AMIDILIB is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with AMIDILIB.  If not, see <http://www.gnu.org/licenses/>.*/



//gcc part
#include <stdio.h>
#include <string.h>
#include <mint/ostruct.h>
#include "INCLUDE/AMIDILIB.H"
#include "INCLUDE/LIST/LIST.H"

/* current version */
typedef struct AMIDI_version {
	U16 major;
	U16 minor;
	U16 patch;
} AMIDI_version;

static AMIDI_version version =
	{ AMIDI_MAJOR_VERSION, AMIDI_MINOR_VERSION, AMIDI_PATCHLEVEL };

/* for saving last running status */
static U8 g_runningStatus;


/* static table with MIDI controller names */
extern const U8 *g_arMIDIcontrollers[];



S16 am_getHeaderInfo(void *pMidiPtr)
{
 sMThd midiInfo;

 memcpy(&midiInfo, pMidiPtr, sizeof(sMThd));

    /* check midi header */
    if(((midiInfo.id)==(ID_MTHD)&&(midiInfo.headLenght==6L)))
	{

        switch(midiInfo.format)
				{
					case 0:
					/* Midi Format 0 detected */
					return (0);
					break;

					case 1:
					/* Midi Format 1 detected */
					return(1);
					break;

					case 2:
					/* Midi Format 2 detected */
					return(2);
					break;

					default:
						/*Error: Unsupported MIDI format */
						return(-2);
					break;
				}

	}
	else if ((midiInfo.id==ID_FORM)||(midiInfo.id==ID_CAT))
        {
            /* possible XMIDI*/


            return(3);

        }
        else
        {
            /* error it's not MIDI file !*/
            return (-1);
        }

return(-1);
}




S16 am_handleMIDIfile(void *pMidiPtr, S16 type, U32 lenght, sSequence_t *pSequence)
{
    S16 iNumTracks=0,iLoop;
   
    U16 iTimeDivision=0;
    U32 ulAddr;
    void *startPtr=pMidiPtr;
    void *endPtr=0L;

    /* calculate end pointer */
    ulAddr=(U32)startPtr+lenght*sizeof(U8);
    endPtr=(void *)ulAddr;

	pSequence->pSequenceName=NULL;		 /* name of the sequence empty string */
	pSequence->ubNumTracks=0;		 /*  */
	pSequence->currentState.ubActiveTrack=0; /* first one from the array */

	/* init sequence table */
	for(iLoop=0;iLoop<AMIDI_MAX_TRACKS;iLoop++){
		/* we will allocate needed track tables when appropriate */
		pSequence->arTracks[iLoop]=NULL;
	}
	
    
    
    switch(type)
    {

        case 0:
            {
            /* handle MIDI type 0 */
            iNumTracks=am_getNbOfTracks(pMidiPtr,type);

            if(iNumTracks!=1)
                {return(-1);} /* invalid number of tracks, there can be only one! */
            else
                {
                    /* prepare our structure */
					pSequence->ubNumTracks=1;	/* one by default */
					
					/* OK! valid number of tracks */
                    /* get time division for timing */
                    iTimeDivision = am_getTimeDivision(pMidiPtr);
                    /* process track data, offset the start pointer a little to get directly to track data and decode MIDI events */
                    startPtr=(void *)((U8 *)startPtr+12);
       
					
					/* Time division handling example, TODO: translate this value to consistent tempo value to use across the whole program */
					am_printTimeDivisionInfo(iTimeDivision);
  

                    while (startPtr!=endPtr)
                    {
                        /* 
							Pointer do midi data, type of midi to preprocess, number of tracks, pointer to the structure in which track data will						    be dumped (or not).  
						*/

						startPtr=processMidiTrackData(startPtr,T_MIDI0,1, pSequence);

                    }

                }
            return(0);
            }
        break;

        case 1:
            {
              /* handle MIDI type 1 */
			  /* several tracks, one sequence */

			  /* prepare our structure */
					pSequence->ubNumTracks=1;	/* one by default */
					iNumTracks=am_getNbOfTracks(pMidiPtr,type);
					iTimeDivision = am_getTimeDivision(pMidiPtr);
                    startPtr=(void *)((U32)startPtr+sizeof(sMThd));
                	
					/* TODO: fill in proper value based on timedivision and PPQ/SMPTE */
						
					
					/* Time division handling example, TODO: translate this value to consistent tempo value to use across the whole program */
					am_printTimeDivisionInfo(iTimeDivision);

                    
					/* create one track list only */
					pSequence->arTracks[0] = (sTrack_t *)malloc(sizeof(sTrack_t));
					/*assert(pCurSequence->arTracks[0]>0);*/

					(pSequence->arTracks[0])->pInstrumentName=NULL;
					(pSequence->arTracks[0])->currTrackState.currentPos=0;
					(pSequence->arTracks[0])->currTrackState.ubVolume=128;                
					//(pSequence->arTracks[0])->currTrackState.ubPlayModeState=0;          /* IDLE state */
					(pSequence->arTracks[0])->currTrackState.ulTimeStep=128;                /* sequence current track tempo */
		
		/* init event list */
		initEventList(&((pSequence->arTracks[0])->trkEventList));

                    
              while (((startPtr!=endPtr)&&(startPtr!=NULL)))
                    {
                        startPtr=processMidiTrackData(startPtr,T_MIDI1, iNumTracks, pSequence);

                    }

            return(0);
            }
        break;

        case 2:
            {
				/* handle MIDI type 2 */
				/* several tracks not tied to each others tracks */

			
				
				iNumTracks=am_getNbOfTracks(pMidiPtr,type);
				iTimeDivision = am_getTimeDivision(pMidiPtr);
				startPtr=(void *)((U32)startPtr+sizeof(sMThd));
				
                
				/* TODO: fill in proper value based on timedivision and PPQ/SMPTE */
				(pSequence->arTracks[0])->currTrackState.ulTimeStep=128;	

				/* Time division handling example, TODO: translate this value to consistent tempo value to use across the whole program */
				am_printTimeDivisionInfo(iTimeDivision);

                while (((startPtr!=endPtr)&&(startPtr!=NULL)))
                {
                  startPtr=processMidiTrackData(startPtr,T_MIDI2,iNumTracks,pSequence);

                }
             return(0);
            }
        break;
		case 3:
            {
                /* handle XMIDI */
                iNumTracks=am_getNbOfTracks(pMidiPtr,type);
                iTimeDivision = am_getTimeDivision(pMidiPtr);

                /* processing (X)MIDI file */
				/* TODO: handle + process */

            return(0);
            }

        break;
		default:;
			/* unsupported file type */
 }
 return(-1);
}

S16 am_getNbOfTracks(void *pMidiPtr, S16 type)
{
 sMThd midiInfo;
 IFF_Chunk xmidiInfo;
 U32 ulNextChunk=0;
 U32 ulChunkOffset=0;
 U8 *Ptr=NULL;

    switch(type)
    {
     case T_MIDI0:
     case T_MIDI1:
     case T_MIDI2:
     {
         memcpy(&midiInfo, pMidiPtr, sizeof(sMThd));

        /* check midi header */
        if(((midiInfo.id)==(ID_MTHD)&&(midiInfo.headLenght==6L)))
        {

            return (midiInfo.nTracks);
        }
     }
     break;

     case T_XMIDI:
     {
        /*TODO: ! not implemented */
     }
     break;
    }

 return (-1);
}

U16 am_getTimeDivision (void *pMidiPtr)
{
    sMThd midiInfo;

    memcpy(&midiInfo, pMidiPtr, sizeof(sMThd));

    /* check midi header */
    if(((midiInfo.id)==(ID_MTHD)&&(midiInfo.headLenght==6L)))
        {


            return (midiInfo.division);
        }
    /* (X)Midi has timing data inside midi eventlist */

 return (0);
}


S16 am_getTrackInfo(void *pMidiPtr, U16 usiTrackNb, sMIDItrackInfo *pTrackInfo)
{
 return(0);
}



void *am_getTrackPtr(void *pMidiPtr,S16 iTrackNum)
{

return NULL;

}

U8 am_calcRolandChecksum(U8 *buf_start, U8 *buf_end)
{
	U8 total = 0 ;
	U8 mask  = 0x7F ;
	while ( buf_start <= buf_end )
	{
		total += *buf_start ;
		buf_start++ ;
	}
	return (0x80 - (total & mask)) & mask ;
}


static U8 g_arMidiBuffer[MIDI_BUFFER_SIZE];


/* Midi buffers system info */
#ifdef _PUREC_
static IOREC g_sOldMidiBufferInfo;
static IOREC *g_psMidiBufferInfo;
#else
static _IOREC g_sOldMidiBufferInfo;
static _IOREC *g_psMidiBufferInfo;

#endif

S16 am_init()
{
	S32 iCounter=0;
            am_setSuperOn();

	   		/* clear our new buffer */
	   		
	   		for(iCounter=0;iCounter<(MIDI_BUFFER_SIZE-1);iCounter++)
	   		{
	   		 g_arMidiBuffer[iCounter]=0x00;
	   		} 
	   		#ifdef _PUREC_
			  g_psMidiBufferInfo=(IOREC*)Iorec(XB_DEV_MIDI);
			#else
			  g_psMidiBufferInfo=(_IOREC*)Iorec(XB_DEV_MIDI);
			#endif
	 		/* copy old MIDI buffer info */
	 		g_sOldMidiBufferInfo.ibuf=(*g_psMidiBufferInfo).ibuf;
	 		g_sOldMidiBufferInfo.ibufsiz=(*g_psMidiBufferInfo).ibufsiz;
	 		g_sOldMidiBufferInfo.ibufhd=(*g_psMidiBufferInfo).ibufhd;
	 		g_sOldMidiBufferInfo.ibuftl=(*g_psMidiBufferInfo).ibuftl;
	 		g_sOldMidiBufferInfo.ibuflow=(*g_psMidiBufferInfo).ibuflow;
	 		g_sOldMidiBufferInfo.ibufhi=(*g_psMidiBufferInfo).ibufhi;

	 		/* set up new MIDI buffer */
	 		(*g_psMidiBufferInfo).ibuf = (char *)g_arMidiBuffer;
	 		(*g_psMidiBufferInfo).ibufsiz = MIDI_BUFFER_SIZE;
	 		(*g_psMidiBufferInfo).ibufhd=0;	        /* first byte index to write */
		 	(*g_psMidiBufferInfo).ibuftl=0;         /* first byte to read(remove) */
	 		(*g_psMidiBufferInfo).ibuflow=(U16)MIDI_LWM;
	 		(*g_psMidiBufferInfo).ibufhi=(U16)MIDI_HWM;
            am_setSuperOff();
 		 	return(1);

}

void am_deinit()
{
    am_setSuperOn();
	/* restore standard MIDI buffer */
 	(*g_psMidiBufferInfo).ibuf=g_sOldMidiBufferInfo.ibuf;
	(*g_psMidiBufferInfo).ibufsiz=g_sOldMidiBufferInfo.ibufsiz;
	(*g_psMidiBufferInfo).ibufhd=g_sOldMidiBufferInfo.ibufhd;
	(*g_psMidiBufferInfo).ibuftl=g_sOldMidiBufferInfo.ibuftl;
 	(*g_psMidiBufferInfo).ibuflow=g_sOldMidiBufferInfo.ibuflow;
	(*g_psMidiBufferInfo).ibufhi=g_sOldMidiBufferInfo.ibufhi;
    am_setSuperOff();
	/* end sequence */
}

void am_dumpMidiBuffer()
{
 U32 counter=0;
#ifdef _PUREC_
 IOREC *g_psMidiBufferInfo;
#else
 _IOREC *g_psMidiBufferInfo;
#endif
  printf("MIDI buffer dump:");
  
  
 for(counter=0;counter<(MIDI_BUFFER_SIZE-1);counter++)
 {
 	
 	if(g_arMidiBuffer[counter]!=0x00)	printf("%x",g_arMidiBuffer[counter]);
 	
 }
  printf("\n");

}


/* at this point pCurSequence should have the info about the type of file that resides in memory,
because we have to know if we have to dump event data to one eventlist or several ones */

/* all the events found in the track will be dumped to the sSequenceState_t structure  */

void * processMidiTrackData(void *startPtr, U32 fileTypeFlag,U32 numTracks, sSequence_t *pCurSequence)
{   
    sChunkHeader header;
    U32 trackCounter=0;
    U32 endAddr=0L;
    U32 ulChunkSize=0;

    printf("Nb of tracks to process: %ld\n",numTracks);

    memcpy(&header, startPtr, sizeof(sChunkHeader));
    startPtr=(U8*)startPtr + sizeof(sChunkHeader);
    
    ulChunkSize=header.headLenght;
    endAddr=(U32)startPtr+header.headLenght;

    if(fileTypeFlag!=T_MIDI2)
	{

		while(( (header.id==ID_MTRK)&&(trackCounter<numTracks)))
		{
			/* we have got track data :)) */
			/* add all of them to given track */ 
			sTrack_t *pTempTrack=pCurSequence->arTracks[0];
			sTrack_t **ppTrack=&pTempTrack;
			
			const void *pTemp=(const void *)endAddr;
			const void **end=&pTemp;
			startPtr=processMIDItrackEvents(&startPtr,end,ppTrack );
			
			/* get next data chunk info */
			memcpy(&header, startPtr,sizeof(sChunkHeader));
		    ulChunkSize=header.headLenght;

			/* omit Track header */
			startPtr=(U8*)startPtr+sizeof(sChunkHeader);
			endAddr=(U32)startPtr+header.headLenght;

			/* increase track counter */
			trackCounter++;
		}
	}
	else 
	{	
		/* handle MIDI 2, multitrack type */
		/* create several track lists according to numTracks */
		
		
			/*  TODO: not finished !*/
			/*assert((pCurSequence->arTracks[trackCounter])->pEventListPtr>0);*/

			/* init event list */
			/*initEventList((pCurSequence->arTracks[trackCounter])->pEventListPtr);*/
			
		
		 /* tracks inited, now insert track data */
		
		trackCounter=0;	/* reset track counter first */

		while(( (header.id==ID_MTRK)&&(trackCounter<numTracks)))
		{
			/* we have got track data :)) */
			/* add all of them to given track */ 
			sTrack_t *pTempTrack=pCurSequence->arTracks[trackCounter];
			sTrack_t **ppTrack=&pTempTrack;
			
			const void *pTemp=(const void *)endAddr;
			const void **end=&pTemp;
			
			startPtr=processMIDItrackEvents(&startPtr,end,ppTrack);
			
			/* get next data chunk info */
			memcpy(&header, startPtr,sizeof(sChunkHeader));
		    ulChunkSize=header.headLenght;

			/* omit Track header */
			startPtr=(U8*)startPtr+sizeof(sChunkHeader);
			endAddr=(U32)startPtr+header.headLenght;

			/* increase track counter */
			trackCounter++;
		}
	
	}
 
	
	return NULL;
}

U8 am_isMidiChannelEvent(U8 byteEvent)
{

    if(( ((byteEvent&0xF0)>=0x80) && ((byteEvent&0xF0)<=0xE0)))
    {return 1;}
    else return 0;
}

U8 am_isMidiRTorSysex(U8 byteEvent)
{

    if( ((byteEvent>=(U8)0xF0)&&(byteEvent<=(U8)0xFF)) )
    {   /* it is! */
        return (1);
    }
    else /*no, it's not! */
        return (0);
}

/* handles the events in tracks and returns pointer to the next midi track */

void *processMIDItrackEvents(void**startPtr, const void **endAddr, sTrack_t **pCurTrack )
{
    U8 *pCmd=((U8 *)(*startPtr));
    U8 ubSize;
    U8 usSwitch=0;
    U16 recallStatus=0;
    U32 delta=0L;
    U32 deltaAll=0L;

    /* TODO: check EOT value, midi 50% track headers are broken, so the web says ;)) */
    while (pCmd!=(*endAddr))
    {

		/*read delta time, pCmd should point to the command data */
		delta=readVLQ(pCmd,&ubSize);
		deltaAll=deltaAll+delta;
		printf("Event: delta %ld \n",deltaAll);

		pCmd=(U8 *)((U32)pCmd+ubSize*sizeof(U8));

		/* handling of running status */
		/* if byte is not from 0x08-0x0E range then recall last running status AND set recallStatus = 1 */
		/* else set recallStatus = 0 and do nothing special */

		ubSize=(*pCmd);

		if( (!(am_isMidiChannelEvent(ubSize))&&(recallStatus==1)&&(!(am_isMidiRTorSysex(ubSize)))))
		{


            /*recall last cmd byte */
            usSwitch=g_runningStatus;
            usSwitch=((usSwitch>>4)&0x0F);



		}
		else
		{
            /* check if the new cmd is the system one*/
            recallStatus=0;

            if((am_isMidiRTorSysex(ubSize)))
            {
                 usSwitch=ubSize;
            }
            else
            {
                usSwitch=ubSize;
                usSwitch=((usSwitch>>4)&0x0F);

            }

		}

		/* decode event and write it to our custom structure */
		switch(usSwitch)
		{
			case EV_NOTE_OFF:
				am_noteOff(&pCmd,&recallStatus, delta, pCurTrack );
			break;
			case EV_NOTE_ON:
				am_noteOn(&pCmd,&recallStatus, delta, pCurTrack );
			break;
			case EV_NOTE_AFTERTOUCH:
				am_noteAft(&pCmd,&recallStatus, delta, pCurTrack );
			break;
			case EV_CONTROLLER:
				am_Controller(&pCmd,&recallStatus, delta, pCurTrack );
			break;
			case EV_PROGRAM_CHANGE:
				am_PC(&pCmd,&recallStatus, delta, pCurTrack );
			break;
			case EV_CHANNEL_AFTERTOUCH:
				am_ChannelAft(&pCmd,&recallStatus, delta, pCurTrack );
			break;
			case EV_PITCH_BEND:
				am_PitchBend(&pCmd,&recallStatus, delta, pCurTrack );
			break;
			case EV_META:
				am_Meta(&pCmd, delta, pCurTrack );
			break;
			case EV_SOX:                          /* SySEX midi exclusive */
				recallStatus=0; 	                /* cancel out midi running status */
				am_Sysex(&pCmd,delta, pCurTrack);
			break;
			case SC_MTCQF:
				recallStatus=0;                        /* Midi time code quarter frame, 1 byte */
				printf("Event: System common MIDI time code qt frame\n");
				pCmd++;
				pCmd++;
			break;
			case SC_SONG_POS_PTR:
				printf("Event: System common Song position pointer\n");
				recallStatus=0;                      /* Song position pointer, 2 data bytes */
				pCmd++;
				pCmd++;
				pCmd++;
			break;
			case SC_SONG_SELECT:              /* Song select 0-127, 1 data byte*/
				printf("Event: System common Song select\n");
				recallStatus=0;
				pCmd++;
				pCmd++;
			break;
			case SC_UNDEF1:                   /* undefined */
			case SC_UNDEF2:                  /* undefined */
				printf("Event: System common not defined\n");
				recallStatus=0;
				pCmd++;
			break;
			case SC_TUNE_REQUEST:             /* tune request, no data bytes */
				printf("Event: System tune request\n");
				recallStatus=0;
				pCmd++;
			break;

			default:
				printf("Event: Unknown type: %d\n",(*pCmd));
				/* unknown event, do nothing or maybe throw error? */
    }
} /*end of decode events loop */
/* return the next track data */
return(pCmd);
}


void am_noteOff(U8 **pPtr,U16 *recallRS,U32 delta, sTrack_t **pCurTrack)
{
    U8 channel=0;
    U8 note=0;
    U8 velocity=0;
	sEventBlock_t tempEvent;
    sNoteOff_EventBlock_t *pEvntBlock=NULL;

	if((*recallRS)==0)
		{
 			/* save last running status */
 			g_runningStatus=*(*pPtr);
			
			tempEvent.uiDeltaTime=delta;
			tempEvent.type=T_NOTEOFF;
			tempEvent.infoBlock=getEventFuncInfo(T_NOTEOFF);
			tempEvent.dataPtr=malloc(tempEvent.infoBlock.size);
			/*assert(tempEvent.dataPtr>0);*/
			pEvntBlock=(sNoteOff_EventBlock_t *)tempEvent.dataPtr;
			pEvntBlock->ubChannelNb=g_runningStatus&0x0F;

			/* now we can recall former running status next time */
			(*recallRS)=1;

			(*pPtr)++;
			channel=(g_runningStatus&0x0F)+1;
			note=*(*pPtr);
			pEvntBlock->eventData.noteNb=*(*pPtr);
			
			/* get parameters */
			(*pPtr)++;
			velocity=*(*pPtr);
			pEvntBlock->eventData.velocity=*(*pPtr);
			
			(*pPtr)++;

			/* add event to list */
			addEvent( &((*pCurTrack)->trkEventList), &tempEvent );
			free(tempEvent.dataPtr);

		}
		else
		{
			/* recall last cmd status */
			/* and get parameters as usual */

			/* get last note info */
			tempEvent.uiDeltaTime=delta;
			tempEvent.type=T_NOTEOFF;
			tempEvent.infoBlock=getEventFuncInfo(T_NOTEOFF);
			tempEvent.dataPtr=malloc(tempEvent.infoBlock.size);
			/*assert(tempEvent.dataPtr>0);*/
			pEvntBlock=(sNoteOff_EventBlock_t *)tempEvent.dataPtr;
			/* save channel */
			pEvntBlock->ubChannelNb=g_runningStatus&0x0F;

			channel=(g_runningStatus&0x0F)+1;
			note=*(*pPtr);
			pEvntBlock->eventData.noteNb=*(*pPtr);

			/* get parameters */
			(*pPtr)++;
			velocity=*(*pPtr);
			pEvntBlock->eventData.velocity=*(*pPtr);
			
			(*pPtr)++;

			/* add event to list */
			addEvent(&((*pCurTrack)->trkEventList), &tempEvent );
			free(tempEvent.dataPtr);
		
		}
		printf("Note off: ");
		printf(" channel: %d ",(g_runningStatus&0x0F)+1);
		printf(" note: %ld ",(*pPtr));
		printf(" vel: %ld \n",(*pPtr));
}

void am_noteOn(U8 **pPtr,U16 *recallRS,U32 delta, sTrack_t **pCurTrack)
{
 U8 channel=0;
 U8 note=0;
 U8 velocity=0;
 sEventBlock_t tempEvent;
 sNoteOn_EventBlock_t *pEvntBlock=NULL;

 if((*recallRS)==0)
 {
    /* save last running status */
    g_runningStatus=*(*pPtr);

	tempEvent.uiDeltaTime=delta;
	tempEvent.type=T_NOTEON;
	tempEvent.infoBlock=getEventFuncInfo(T_NOTEON);
	tempEvent.dataPtr=malloc(tempEvent.infoBlock.size);
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

	/* add event to list */
		addEvent(&((*pCurTrack)->trkEventList), &tempEvent );
		free(tempEvent.dataPtr);

 }
 else{
    /* get last note info */
    channel=(g_runningStatus&0x0F)+1;

	tempEvent.uiDeltaTime=delta;
	tempEvent.type=T_NOTEON;
	tempEvent.infoBlock=getEventFuncInfo(T_NOTEON);
	tempEvent.dataPtr=malloc(tempEvent.infoBlock.size);
	/*assert(tempEvent.dataPtr>0);*/
	pEvntBlock=(sNoteOn_EventBlock_t *)tempEvent.dataPtr;

	pEvntBlock->ubChannelNb=g_runningStatus&0x0F;

    note=*(*pPtr);
	pEvntBlock->eventData.noteNb=*(*pPtr);
    /* get parameters */
    (*pPtr)++;
    velocity=*(*pPtr);
	pEvntBlock->eventData.velocity=*(*pPtr);
	
    (*pPtr)++;

	/* add event to list */
		addEvent(&((*pCurTrack)->trkEventList), &tempEvent );
		free(tempEvent.dataPtr);
 }
 /* print and handle */
    printf("Note on: ",channel);
    printf(" channel: %d ",channel);
    printf(" note: %d ",note);
    printf(" vel: %d \n",velocity);
}

void am_noteAft(U8 **pPtr,U16 *recallRS,U32 delta, sTrack_t **pCurTrack)
{
U8 noteNb=0;
U8 pressure=0;
sEventBlock_t tempEvent;
sNoteAft_EventBlock_t *pEvntBlock=NULL;

 if((*recallRS)==0)
 {
    /* save last running status */
    g_runningStatus=*(*pPtr);
	
	tempEvent.uiDeltaTime=delta;
	tempEvent.type=T_NOTEAFT;
	tempEvent.infoBlock=getEventFuncInfo(T_NOTEAFT);
	tempEvent.dataPtr=malloc(tempEvent.infoBlock.size);
	/*assert(tempEvent.dataPtr>0);*/
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
	/* add event to list */
		addEvent(&((*pCurTrack)->trkEventList), &tempEvent );
		free(tempEvent.dataPtr);
 }
 else
    {
        /* get parameters */
		tempEvent.uiDeltaTime=delta;
		tempEvent.type=T_NOTEAFT;
		tempEvent.infoBlock=getEventFuncInfo(T_NOTEAFT);
		tempEvent.dataPtr=malloc(tempEvent.infoBlock.size);
		/*assert(tempEvent.dataPtr>0);*/
		pEvntBlock=(sNoteAft_EventBlock_t *)tempEvent.dataPtr;
		pEvntBlock->ubChannelNb=g_runningStatus&0x0F;

        noteNb=*(*pPtr);
		pEvntBlock->eventData.noteNb=*(*pPtr);
        (*pPtr)++;
        pressure=*(*pPtr);
		pEvntBlock->eventData.pressure=*(*pPtr);
        (*pPtr)++;
		
		/* add event to list */
		addEvent(&((*pCurTrack)->trkEventList), &tempEvent );
		free(tempEvent.dataPtr);

    }
     printf(" Note Aftertouch, note: %d, pressure: %d\n",noteNb,pressure);

}

void am_Controller(U8 **pPtr,U16 *recallRS,U32 delta, sTrack_t **pCurTrack)
{
    U8 channelNb=0;
    U8 controllerNb=0;
    U8 value=0;
	sEventBlock_t tempEvent;
	sController_EventBlock_t *pEvntBlock=NULL;

    if((*recallRS)==0)
    {
        /* save last running status */
        g_runningStatus=*(*pPtr);
        /* now we can recall former running status next time */
        (*recallRS)=1;

		tempEvent.uiDeltaTime=delta;
		tempEvent.type=T_CONTROL;
		tempEvent.infoBlock=getEventFuncInfo(T_CONTROL);
		tempEvent.dataPtr=malloc(tempEvent.infoBlock.size);
		/*assert(tempEvent.dataPtr>0);*/
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
		
		/* add event to list */
		addEvent(&((*pCurTrack)->trkEventList), &tempEvent );
		free(tempEvent.dataPtr);
    }
    else
    {
        
        channelNb=g_runningStatus&0x0F;
		
		tempEvent.uiDeltaTime=delta;
		tempEvent.type=T_CONTROL;
		tempEvent.infoBlock=getEventFuncInfo(T_CONTROL);
		tempEvent.dataPtr=malloc(tempEvent.infoBlock.size);
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

		/* add event to list */
		addEvent(&((*pCurTrack)->trkEventList), &tempEvent  );
		free(tempEvent.dataPtr);

    }

    printf("Controller channel: %d, nb:%d, name:%s value: %d\n",channelNb+1, controllerNb,getMIDIcontrollerName(controllerNb), value);


}

void am_PC(U8 **pPtr,U16 *recallRS,U32 delta, sTrack_t **pCurTrack)
{
	U8 channel=0;
	U8 PN=0;
	sEventBlock_t tempEvent;
	sPrgChng_EventBlock_t *pEvntBlock=NULL;

     if((*recallRS)==0)
	{
        /* save last running status */
        g_runningStatus=*(*pPtr);
		
		/* now we can recall former running status next time */
        (*recallRS)=1;

		channel=(g_runningStatus&0x0F)+1;

		tempEvent.uiDeltaTime=delta;
		tempEvent.type=T_PRG_CH;
		tempEvent.infoBlock=getEventFuncInfo(T_PRG_CH);
		tempEvent.dataPtr=malloc(tempEvent.infoBlock.size);
		/*assert(tempEvent.dataPtr>0);*/
		pEvntBlock=(sPrgChng_EventBlock_t *)tempEvent.dataPtr;

        

		pEvntBlock->ubChannelNb=g_runningStatus&0x0F;
        (*pPtr)++;
        /* get parameters */
        PN=*(*pPtr);
		pEvntBlock->eventData.programNb=*(*pPtr);
		(*pPtr)++;
		/* add event to list */
		addEvent(&((*pCurTrack)->trkEventList), &tempEvent );
		free(tempEvent.dataPtr);
    }
    else
    {
         /* get last PC status */
          channel=(g_runningStatus&0x0F)+1;
		  tempEvent.uiDeltaTime=delta;
		  tempEvent.type=T_PRG_CH;
		  tempEvent.infoBlock=getEventFuncInfo(T_PRG_CH);
		  tempEvent.dataPtr=malloc(tempEvent.infoBlock.size);
		  /*assert(tempEvent.dataPtr>0);*/
		  pEvntBlock=(sPrgChng_EventBlock_t *)tempEvent.dataPtr;
		  pEvntBlock->ubChannelNb=g_runningStatus&0x0F;

          PN=*(*pPtr);
		  pEvntBlock->eventData.programNb=*(*pPtr);
         
		 /* get parameters */
		  (*pPtr)++;
		
		 /* add event to list */
		addEvent(&((*pCurTrack)->trkEventList), &tempEvent );
		free(tempEvent.dataPtr);
    }

    printf("Program change: ");
    printf(" channel: %d ",channel);
    printf(" program nb: %d \n",PN);

}

void am_ChannelAft(U8 **pPtr,U16 *recallRS,U32 delta, sTrack_t **pCurTrack)
{
U8 channel=0;
U8 param=0;
sEventBlock_t tempEvent;
sChannelAft_EventBlock_t *pEvntBlock=NULL;

    if((*recallRS)==0)
    {
        /* save last running status */
        g_runningStatus=*(*pPtr);
        /* now we can recall former running status next time */
        (*recallRS)=1;

		tempEvent.uiDeltaTime=delta;
		tempEvent.type=T_CHAN_AFT;
		tempEvent.infoBlock=getEventFuncInfo(T_CHAN_AFT);
		tempEvent.dataPtr=malloc(tempEvent.infoBlock.size);
		/*assert(tempEvent.dataPtr>0);*/
		pEvntBlock=(sChannelAft_EventBlock_t *)tempEvent.dataPtr;
		pEvntBlock->ubChannelNb=(g_runningStatus&0x0F);
        
		/* get parameters */
        (*pPtr)++;
        param=*(*pPtr);
		pEvntBlock->eventData.pressure=*(*pPtr);
        (*pPtr)++;

		/* add event to list */
		addEvent(&((*pCurTrack)->trkEventList), &tempEvent );
		free(tempEvent.dataPtr);

    }
    else
    {
		tempEvent.uiDeltaTime=delta;
		tempEvent.type=T_CHAN_AFT;
		tempEvent.infoBlock=getEventFuncInfo(T_CHAN_AFT);
		tempEvent.dataPtr=malloc(tempEvent.infoBlock.size);
	/*		assert(tempEvent.dataPtr>0);*/
		pEvntBlock=(sChannelAft_EventBlock_t *)tempEvent.dataPtr;
		pEvntBlock->ubChannelNb=(g_runningStatus&0x0F);

        /* get parameters */
        param=*(*pPtr);
		pEvntBlock->eventData.pressure=*(*pPtr);
        (*pPtr)++;

		/* add event to list */
		addEvent(&((*pCurTrack)->trkEventList), &tempEvent );
		free(tempEvent.dataPtr);
    }
    printf("Channel aftertouch, pressure: %d\n",param);

}

void am_PitchBend(U8 **pPtr,U16 *recallRS,U32 delta, sTrack_t **pCurTrack)
{
U8 MSB=0;
U8 LSB=0;
sEventBlock_t tempEvent;
sPitchBend_EventBlock_t *pEvntBlock=NULL;

    if((*recallRS)==0)
    {
        /* save last running status */
        g_runningStatus=*(*pPtr);

        /* now we can recall former running status next time */
        (*recallRS)=1;

		tempEvent.uiDeltaTime=delta;
		tempEvent.type=T_PITCH_BEND;
		tempEvent.infoBlock=getEventFuncInfo(T_PITCH_BEND);
		tempEvent.dataPtr=malloc(tempEvent.infoBlock.size);
		/*assert(tempEvent.dataPtr>0);*/
		pEvntBlock=(sPitchBend_EventBlock_t *)tempEvent.dataPtr;
		pEvntBlock->ubChannelNb=(g_runningStatus&0x0F);

        (*pPtr)++;
        LSB=*(*pPtr);
		pEvntBlock->eventData.LSB=*(*pPtr);
        /* get parameters */
        (*pPtr)++;
        MSB=*(*pPtr);
		pEvntBlock->eventData.MSB=*(*pPtr);
        (*pPtr)++;
		
		/* add event to list */
		addEvent(&((*pCurTrack)->trkEventList), &tempEvent );
		free(tempEvent.dataPtr);
    }
    else
    {
        g_runningStatus;    /* get last PC status */

		tempEvent.uiDeltaTime=delta;
		tempEvent.type=T_PITCH_BEND;
		tempEvent.infoBlock=getEventFuncInfo(T_PITCH_BEND);
		tempEvent.dataPtr=malloc(tempEvent.infoBlock.size);
		/*assert(tempEvent.dataPtr>0);*/
		pEvntBlock=(sPitchBend_EventBlock_t *)tempEvent.dataPtr;
		pEvntBlock->ubChannelNb=(g_runningStatus&0x0F);

        /* get parameters */
        LSB=*(*pPtr);
		pEvntBlock->eventData.LSB=*(*pPtr);
        (*pPtr)++;
        MSB=*(*pPtr);
		pEvntBlock->eventData.MSB=*(*pPtr);
        (*pPtr)++;
		
		/* add event to list */
		addEvent(&((*pCurTrack)->trkEventList), &tempEvent );
		free(tempEvent.dataPtr);
    }
 printf("Pitch bend, LSB: %d, MSB:%d\n",LSB,MSB);
}

void am_Sysex(U8 **pPtr,U32 delta, sTrack_t **pCurTrack)
{
  U32 ulCount=0L;
  sEventBlock_t tempEvent;
 
  printf("SOX: ");

    while( (*(*pPtr))!=EV_EOX)
    {
     printf("%x ",*(*pPtr));
     (*pPtr)++;
      /*count Sysex msg data bytes */
      ulCount++;
    }
    printf(" EOX, size: %ld\n",ulCount);
}

void am_Meta(U8 **pPtr,U32 delta, sTrack_t **pCurTrack)
{
 /* TODO: maybe move these variables to static/global area and/or replace them with register vars for speed ?*/
 U8 ubLenght,ubVal,ubSize=0;
 U8 ulVal[3]={0};   /* for retrieving set tempo info */
 U8 param1=0,param2=0;
 U32 addr;
 U8 textBuffer[128]={0};
 sSMPTEoffset SMPTEinfo;
 sTimeSignature timeSign;
 sEventBlock_t tempEvent;

 printf(" Meta event: ");

 /*get meta event type */
 (*pPtr)++;
 ubVal=*(*pPtr);

 switch(ubVal)
 {
    case MT_SEQ_NB:
        printf("Sequence nb: ");
        (*pPtr)++;
        ubLenght=(*(*pPtr));
        printf("%d\n", ubLenght);
        (*pPtr)++;
        addr=((U32)(*pPtr))+ubLenght*sizeof(U8);
        *pPtr=(U8*)addr;
    break;
    case MT_TEXT:
        printf("Text:");
        (*pPtr)++;
        ubLenght=readVLQ((*pPtr),&ubSize);
        printf("meta size: %d ",ubLenght);
        /* set to the start of the string */
        (*pPtr)++;
        memcpy(textBuffer, (*pPtr),ubLenght*sizeof(U8) );
        (*pPtr)=((*pPtr)+ubLenght);
        printf("%s \n",textBuffer);
    break;
    case MT_COPYRIGHT:
        printf("Copyright: ");
        (*pPtr)++;
        ubLenght=readVLQ((*pPtr),&ubSize);
        printf("meta size: %d ",ubLenght);
        /* set to the start of the string */
        (*pPtr)++;
        memcpy(textBuffer, (*pPtr),ubLenght*sizeof(U8) );
        (*pPtr)=((*pPtr)+ubLenght);
        printf("%s \n",textBuffer);
    break;
    case MT_SEQNAME:
        printf("Sequence name: ");
        (*pPtr)++;
        ubLenght=readVLQ((*pPtr),&ubSize);
        /* set to the start of the string */
        (*pPtr)++;
        memcpy(textBuffer, (*pPtr),ubLenght*sizeof(U8) );
        (*pPtr)=((*pPtr)+ubLenght);
        printf("meta size: %d ",ubLenght);
        printf("%s \n",textBuffer);
    break;
    case MT_INSTRNAME:
        printf("Instrument name: ");
        (*pPtr)++;
        ubLenght=readVLQ((*pPtr),&ubSize);
        printf("meta size: %d",ubLenght);
        /* set to the start of the string */
        (*pPtr)++;
        memcpy(textBuffer, (*pPtr),ubLenght*sizeof(U8) );
        (*pPtr)=((*pPtr)+ubLenght);
        printf("%s \n",textBuffer);
    break;
    case MT_LYRICS:
        printf("Lyrics: ");
        (*pPtr)++;
        ubLenght=readVLQ((*pPtr),&ubSize);
        printf("meta size: %d ",ubLenght);
        /* set to the start of the string */
        (*pPtr)++;
        memcpy(textBuffer, (*pPtr),ubLenght*sizeof(U8) );
        (*pPtr)=((*pPtr)+ubLenght);
        printf("%s \n",textBuffer);
    break;

    case MT_MARKER:
        printf("Marker: ");
        (*pPtr)++;
        ubLenght=readVLQ((*pPtr),&ubSize);
        /* set to the start of the string */
        (*pPtr)++;
        memcpy(textBuffer, (*pPtr),ubLenght*sizeof(U8) );
        (*pPtr)=((*pPtr)+ubLenght);
        printf("meta size: %d ",ubLenght);
        printf("%s \n",textBuffer);
    break;
    case MT_CUEPOINT:
        printf("Cuepoint\n");
        (*pPtr)++;
        ubLenght=readVLQ((*pPtr),&ubSize);
        /* set to the start of the string */
        (*pPtr)++;
        memcpy(textBuffer, (*pPtr),ubLenght*sizeof(U8) );
        (*pPtr)=((*pPtr)+ubLenght);
        printf("meta size: %d ",ubLenght);
        printf("%s \n",textBuffer);
    break;

    case MT_PROGRAM_NAME:
        /* program(patch) name */
        printf("Program (patch) name: ");
        (*pPtr)++;
        ubLenght=readVLQ((*pPtr),&ubSize);

        /* set to the start of the string */
        (*pPtr)++;
        memcpy(textBuffer, (*pPtr),ubLenght*sizeof(U8) );
        (*pPtr)=((*pPtr)+ubLenght);
        printf("meta size: %d ",ubLenght);
        printf("%s \n",textBuffer);
    break;
    case MT_DEVICE_NAME:
        /* device (port) name */
        printf("Device (port) name: ");
        (*pPtr)++;
        ubLenght=readVLQ((*pPtr),&ubSize);

        /* set to the start of the string */
        (*pPtr)++;
        memcpy(textBuffer, (*pPtr),ubLenght*sizeof(U8) );
        (*pPtr)=((*pPtr)+ubLenght);
        printf("meta size: %d ",ubLenght);
        printf("%s \n",textBuffer);
    break;
    case MT_CH_PREFIX:
        printf("Channel prefix\n");
        (*pPtr)++;
        ubLenght=(*(*pPtr));
        (*pPtr)++;
        addr=((U32)(*pPtr))+ubLenght*sizeof(U8);
        *pPtr=(U8*)addr;
    break;
    case MT_MIDI_CH: /* obsolete! just ignore */
        (*pPtr)++;
        /*get size */
        ubLenght=(*(*pPtr));
        (*pPtr)++;

        /*get port nb*/
        printf("Midi channel nb: %d\n",*(*pPtr));
        (*pPtr)++;
    break;
    case MT_MIDI_PORT: /* obsolete! just ignore */
        (*pPtr)++;
        /*get size */
        ubLenght=(*(*pPtr));
        (*pPtr)++;

        /*get port nb*/
        printf("Midi port nb: %d\n",*(*pPtr));
        (*pPtr)++;
    break;
    case MT_EOT:
        printf("End of track\n");
        (*pPtr)++;
        ubLenght=(*(*pPtr));
        (*pPtr)++;
        addr=((U32)(*pPtr))+ubLenght*sizeof(U8);
        *pPtr=(U8*)addr;
    break;
    case MT_SET_TEMPO:
        /* sets tempo in track, should be in the first track, if not 120 BPM is assumed */
		printf("Set tempo: ");
	
		(*pPtr)++;
        ubLenght=(*(*pPtr));
         (*pPtr)++;
        /* get those 3 bytes */
        memcpy(ulVal, (*pPtr),ubLenght*sizeof(U8) );

        addr=((U32)(*pPtr))+ubLenght*sizeof(U8);
        *pPtr=(U8*)addr;
        printf("0x%x%x%x ms per MIDI quarter-note\n", ulVal[0],ulVal[1],ulVal[2]);
    break;
    case MT_SMPTE_OFFSET:
        printf("SMPTE offset:\n");
        (*pPtr)++;
        ubLenght=(*(*pPtr));
        (*pPtr)++;
        memcpy (&SMPTEinfo,(*pPtr),sizeof(sSMPTEoffset));

        addr=((U32)(*pPtr))+ubLenght*sizeof(U8);
        *pPtr=(U8*)addr;
    /* print out info */
        printf("hr: %d\n",SMPTEinfo.hr);
        printf("mn: %d\n",SMPTEinfo.mn);
        printf("se: %d\n",SMPTEinfo.fr);
        printf("fr: %d\n",SMPTEinfo.fr);
        printf("ff: %d\n",SMPTEinfo.ff);
    break;
    case MT_TIME_SIG:
        printf("Time signature:\n");
        (*pPtr)++;
        ubLenght=(*(*pPtr));
        (*pPtr)++;

        memcpy (&timeSign,(*pPtr),sizeof(sTimeSignature));
        addr=((U32)(*pPtr))+ubLenght*sizeof(U8);
        *pPtr=(U8*)addr;
    /* print out info */
        printf("nn: %d\n",timeSign.nn);
        printf("dd: %d\n",timeSign.dd);
        printf("cc: %d\n",timeSign.cc);
        printf("bb: %d\n",timeSign.bb);
    break;
    case MT_KEY_SIG:
        printf("Key signature: ");
        (*pPtr)++;
        ubLenght=(*(*pPtr));
        (*pPtr)++;
        param1=(*(*pPtr));
        (*pPtr)++;
        param2=(*(*pPtr));

        if (param2==0) {printf("scale: major ");}
        else if(param2==1) {printf("scale: minor ");}
         else { printf("error: wrong key signature scale. "); }

        if(param1==0)
            {printf("Key of C\n");}
        else if (((S8)param1==-1))
            {printf("1 flat\n");}
        else if(((S8)param1)==1)
            {printf("1 sharp\n");}
        else if ((param1>1&&param1<=7))
            {printf(" %d sharps\n",param1);}
        else if (( ((S8)param1)<-1&& ((S8)param1)>=-7))
            {printf(" %d flats\n",(U32)param1);}
        else {printf(" error: wrong key signature. \n",param1);}
        (*pPtr)++;

    break;
    case MT_SEQ_SPEC:
        printf("Sequencer specific data.\n");
        (*pPtr)++;
        ubLenght=(*(*pPtr));
        (*pPtr)++;
        addr=((U32)(*pPtr))+ubLenght*sizeof(U8);
        *pPtr=(U8*)addr;
    break;
    default:
        (*pPtr)++;
        ubLenght=(*(*pPtr));
        /* we should put here assertion failed or something with "send this file to author" message */
        /* file also could be broken */
        printf("Unknown meta event id: %d, size: %d parameters: %ld %\n",ubVal,*(*pPtr));
        (*pPtr)=(*pPtr)+ubLenght;
    break;
 }

}


U32 readVLQ(U8 *pChar,U8 *ubSize)
{

     U32 value=0;
     U8 c=0;
     (*ubSize)=0;
    value = (*pChar);
    if ( (value & 0x80) )
    {
       value &= 0x7F;

        /* get next byte */
       pChar++;
       (*ubSize)++;

       do
       {

         value = (value << 7);
         c = (*pChar);
         value = value + (c&0x7F);

         pChar++;
          (*ubSize)++;
       } while (c & 0x80);
    }
    else
        {
            (*ubSize)++;
        }

    return(value);

}

/* combine bytes function for pitch bend */
U16 combineBytes(U8 bFirst, U8 bSecond)
{
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

/* gets info about connected devices via MIDI interface */


const S8 *getConnectedDeviceInfo(void)
{
  /*  request on all channels */
  
  /*static U8 getInfoSysEx[]={0xF0,ID_ROLAND,GS_DEVICE_ID,GS_MODEL_ID,0x7E,0x7F,0x06,0x01,0x00,0xF7}; */
  U8 getInfoSysEx[]={0xF0,0x41,0x10,0x42,0x7E,0x7F,0x06,0x01,0x00,0xF7};
  U8 channel=0x00;
  S32 data;
  
  /* calculate checksum */
  /*getInfoSysEx[5]=am_calcRolandChecksum(&getInfoSysEx[2],&getInfoSysEx[4]);*/
  
  for(channel=0;channel<0x7f;channel++)
  {
  	getInfoSysEx[5]=channel;
	getInfoSysEx[8]=am_calcRolandChecksum(&getInfoSysEx[5],&getInfoSysEx[7]);  
 	 
 	MIDI_SEND_DATA(10,(void *)getInfoSysEx); 
    am_dumpMidiBuffer();
    printf("Sysex channel %d 0x",channel);
    
  	while(MIDI_DATA_READY)
  	{
  		data=GET_MIDI_DATA;
  
  		printf("%x",((U32)data));
  	}
 	printf("\n");
  am_dumpMidiBuffer();
  }
  
 
 return NULL;
}





/* function for calculating tempo */
/* called each time tempo is changed returned value is assigned to TimeStep value in sequence */
/* TODO: rewrite FPU version in asm in 060 and maybe 030 version */

/* BPM - beats per minute (tempo of music) */
/* UPS - update interval (updates per second) */
/* music resolution are in PPQ */

U32  am_calculateTimeStep(U16 qpm, U16 ppq, U16 ups)
{
    U32 ppu;
    U32 temp;
    temp=(U32)qpm*(U32)ppq;
    if(temp<0x10000){
        ppu=((temp*0x10000)/60)/(U32)ups;
    }
    else{
        ppu=((temp/60)*0x10000)/(U32)ups;
    }
return ppu;
}

/* function for calculating tempo (float version) */
/* called each time tempo is changed returned value is assigned to TimeStep value in sequence */
/* BPM - beats per minute (tempo of music) */
/* UPS - update interval (updates per second) */
/* music resolution are in PPQ */

float  am_calculateTimeStepFlt(U16 qpm, U16 ppq, U16 ups)
{
    float ppu;
    float temp;
    ppu=(float)qpm*(float)ppq;
    temp=(temp/ups)/60.0f;
   
 return ppu;
}




/* support functions:
    BPM = 60,000,000/MicroTempo
    MicrosPerPPQN = MicroTempo/TimeBase
    MicrosPerMIDIClock = MicroTempo/24
    PPQNPerMIDIClock = TimeBase/24
    MicrosPerSubFrame = 1000000 * Frames * SubFrames
    SubFramesPerQuarterNote = MicroTempo/(Frames * SubFrames)
    SubFramesPerPPQN = SubFramesPerQuarterNote/TimeBase
    MicrosPerPPQN = SubFramesPerPPQN * Frames * SubFrames
*/

void am_printTimeDivisionInfo(U16 timeDivision)
{
	 U8 subframe=0;
	
	if(timeDivision&0x8000)
       {
        /* SMPTE */
        timeDivision&=0x7FFF;
        subframe=timeDivision>>7;

        printf("Timing (SMPTE): %x, %d\n", subframe,(timeDivision&0x00FF));

	   }
      else
       {
        /* PPQN */
        printf("Timing (PPQN): %d (0x%x)\n", timeDivision,timeDivision);

       }
}


