/*  Copyright 2008, 2009 Pawel Goralski
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "INCLUDE/AMIDILIB.H"
#include "INCLUDE/LIST/LIST.H"

/* test midi file to load */
/*#define MIDI_FILE "..\\TUNES\\ULTIMA30.MID"*/
#define MIDI_FILE "ULTIMA01.MID"
#define XMIDI_FILE "TUNES/UWR10.XMI"


extern "C"{
  static U8 messBuf[256]; /* for error buffer  */
  static BOOL CON_LOG;
}

/**
 * main program entry
 */
void VLQtest(void);
void memoryCheck(void);
void playMidi(sSequence_t *pMidiSequence);

int main(void){
  
    void *pMidi=NULL;
    U16 iRet=0;
    S16 iError=0;
    
    VLQtest();
    memoryCheck();

    /* init library */
    iError=am_init();
    
    /* get connected device info */
    getConnectedDeviceInfo();
    U32 ulFileLenght=0L;
    pMidi=loadFile((U8 *)MIDI_FILE, PREFER_TT, &ulFileLenght);
    
    sSequence_t midiTune;
    if(pMidi!=NULL){

     sprintf((char *)messBuf, "Midi file loaded, size: %u bytes.\n",(unsigned int)ulFileLenght);
     am_log(messBuf,CON_LOG);
     /* process MIDI*/
     /* check midi header */
     iRet=am_getHeaderInfo(pMidi);

     if(iRet>0){
      // no error
      switch(iRet){
	case 0:
	case 1:
	case 2:
	case 3:
	/* handle file */
	  sprintf((char *)messBuf, "Preparing MIDI file for replay\n");
	  am_log(messBuf,CON_LOG);
	  iError=am_handleMIDIfile(pMidi, iRet, ulFileLenght,&midiTune);
	break;

	default:
	  /* unknown error, do nothing */
	  fprintf(stderr, "Unknown error ...\n");
	  iError=1;
	break;
     }
     
    }else{
      if(iRet== -1){
       /* not MIDI file, do nothing */
       fprintf(stderr, "It's not valid MIDI file...\n");
      } else if(iRet==-2){
	/* unsupported MIDI type format, do nothing*/
	fprintf(stderr, "Unsupported MIDI file format...\n");
      }
      iError=1;
    }
     /* free up buffer, we don't need it anymore */
      Mfree(pMidi);pMidi=NULL;
    
    
    if(iError!=1){
     /* play preloaded tune */
        playMidi(&midiTune);
    }
     
    } /* MIDI loading failed */
    else{
      fprintf(stderr, "Error: Couldn't read file...\n");
      return(-1);
    }

    /* turn off all notes on channels 0-15 */
    am_allNotesOff(16);
   
    /* clean up, free internal library buffers etc..*/
    am_deinit();
    myRoutine();	/* vasm test :D */
     void installTickCounter();
 
    /*interrupt handler test */
    for(;;){
      printf("%u\n",(unsigned int)counter);
    }
     void deinstallTickCounter();
 return (0);
}

void VLQtest(void){
/* VLQ test */
    U32 val[]={0x00, 0x7F,0x8100,0xC000,0xFF7F,0x818000, 0xFFFF7F,0x81808000,0xC0808000,0xFFFFFF7F };
    U32 result=0,iCounter;
    U8 *pValPtr=NULL;
    U8 valsize;
    
    for (iCounter=0;iCounter<10;iCounter++)   {
        pValPtr=(U8 *)(&val[iCounter]);
        while((*pValPtr)==0x00)
        {pValPtr++;}
        valsize=0;result=0;
        result= readVLQ(pValPtr,&valsize);
     	sprintf((char *)messBuf, "VLQ value:%x, decoded: %x, size: %d\n",(unsigned int)val[iCounter], (unsigned int)result, valsize );
	am_log(messBuf,CON_LOG);
    }
    /* End of VLQ test */
}

void memoryCheck(void){
    U32 mem=0;
    
    /* mem tst */
    mem=getFreeMem(ST_RAM);
    mem=getFreeMem(TT_RAM);
    mem=getFreeMem(PREFER_ST);
    mem=getFreeMem(PREFER_TT);
}

void playMidi(sSequence_t *pMidiSequence){
  U32 currDelta=0,lastDelta=0;
  const sEventList *pMyEvent=NULL;	
  
  /* get timing info */
  /* the smallest tick is trackTempo/td */
  U32 trackTempo=pMidiSequence->arTracks[0]->currTrackState.ulTrackTempo;
  U16 td=pMidiSequence->uiTimeDivision;
  double tick=(double)trackTempo/(double)td;
  
  sprintf((char *)messBuf, "\nplayMidi: time division: %d[MPQ], track tempo:%u [ms], tick: %f\n",td,(unsigned int)trackTempo,tick);
  am_log(messBuf,CON_LOG);
  /* get first event */
  pMyEvent= &((pMidiSequence->arTracks[0])->trkEventList); 
  /* play our sequence - send all events  */		      
  /*printEventList( &pMyEvent );*/
  
  /* deltas are relative to the last event */
  sprintf((char *)messBuf, "Sending all events with delta: %u\n", (unsigned int)currDelta);
  am_log(messBuf,CON_LOG);
  while((currDelta=sendMidiEvents(lastDelta, &pMyEvent))){
    sprintf((char *)messBuf, "Sending all events with delta: %u\n", (unsigned int)currDelta);
    am_log(messBuf,CON_LOG);
    lastDelta=currDelta;
  }
  
  sprintf((char *)messBuf,"File processed successfully. ");
  am_log(messBuf,CON_LOG);
  
}
