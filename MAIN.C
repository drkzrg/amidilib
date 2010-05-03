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
#include <time.h>

#include "INCLUDE/AMIDILIB.H"
#include "INCLUDE/LIST/LIST.H"

/* test midi file to load */
/*#define MIDI_FILE "..\\TUNES\\ULTIMA30.MID"*/
#define MIDI_FILE "ULTIMA01.MID"
#define XMIDI_FILE "TUNES/UWR10.XMI"

extern "C"{
 extern void installTA(void);
 extern void installTickCounter(void);
 extern void deinstallTickCounter(void);
 static U8 messBuf[256]; /* for error buffer  */
}
 extern volatile U32 counter;
 extern volatile U32 cA;

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

     amTrace((const U8*)"Midi file loaded, size: %u bytes.\n",(unsigned int)ulFileLenght);
     
     /* process MIDI*/
     /* check midi header */
     iRet=am_getHeaderInfo(pMidi);
     double ms;
     printf("Please wait...\n");
     
     switch(iRet){
	
	case T_MIDI0:{ /* handle file */
	  amTrace((const U8*)"Preparing MIDI type 0 file for replay\n");
	  
	  clock_t begin=clock();
	  iError=am_handleMIDIfile(pMidi, T_MIDI0, ulFileLenght,&midiTune);
	  clock_t end=clock();
	  
	  ms=am_diffclock(end,begin);
	  if(iError==0){
	    printf("MIDI file parsed in ~%4.2f[sec]/~%4.2f[min](%6.4f [ms])\n",ms/1000.0f,ms/1000.0f/60.0f,ms);
	    amTrace((const U8*)"MIDI file parsed in ~%4.2f[sec]/~%4.2f[min](%6.4f [ms])\n",ms/1000.0f,ms/1000.0f/60.0f,ms);
	  }else{
	    amTrace((const U8*)"Error while parsing. Exiting... \n");
	  }
	}
	break;
	case T_MIDI1:{ /* handle file */
	  amTrace((const U8*)"Preparing MIDI type 1 file for replay\n");
	  
	  
	  clock_t begin=clock();
	  iError=am_handleMIDIfile(pMidi, T_MIDI1, ulFileLenght,&midiTune);
	  clock_t end=clock();
	  
	  ms=am_diffclock(end,begin);
	  if(iError==0){
	    amTrace((const U8*)"MIDI file parsed in ~%4.2f[sec]/~%4.2f[min](%6.4f [ms])\n",ms/1000.0f,ms/1000.0f/60.0f,ms);
	    printf("MIDI file parsed in ~%4.2f[sec]/~%4.2f[min](%6.4f [ms])\n",ms/1000.0f,ms/1000.0f/60.0f,ms);
	    
	    
	  }else{
	    amTrace((const U8*)"Error while parsing. Exiting... \n");
	    
	  }
	  
	}break;
	
	case T_MIDI2:{ /* handle file */
	  amTrace((const U8*)"Preparing MIDI type 2 file for replay\n");
	  
	  
	   clock_t begin=clock();
	  iError=am_handleMIDIfile(pMidi, T_MIDI2, ulFileLenght,&midiTune);
	  clock_t end=clock();
	  
	   ms=am_diffclock(end,begin);
	  if(iError==0){
	    amTrace((const U8*)"MIDI file parsed in ~%4.2f[sec]/~%4.2f[min](%6.4f [ms])\n",ms/1000.0f,ms/1000.0f/60.0f,ms);
	    printf("MIDI file parsed in ~%4.2f[sec]/~%4.2f[min](%6.4f [ms])\n",ms/1000.0f,ms/1000.0f/60.0f,ms);
	    
	    
	  }else{
	    amTrace((const U8*)"Error while parsing. Exiting... \n");
	    
	  }
	  
	}
	break;
	case T_XMIDI:
	/* handle file */{
	  amTrace((const U8*)"Preparing XMIDI file for replay\n");
	  
	  clock_t begin=clock();
	  iError=am_handleMIDIfile(pMidi, iRet, ulFileLenght,&midiTune);
	  clock_t end=clock();
	  
	  ms=am_diffclock(end,begin);
	  
	  if(iError==0){
	    amTrace((const U8*)"MIDI file parsed in ~%4.2f[sec]/~%4.2f[min](%6.4f [ms])\n",ms/1000.0f,ms/1000.0f/60.0f,ms);
	    printf("MIDI file parsed in ~%4.2f[sec]/~%4.2f[min](%6.4f [ms])\n",ms/1000.0f,ms/1000.0f/60.0f,ms);
	    
	    
	  }else{
	    amTrace((const U8*)"Error while parsing. Exiting... \n");
	  }
	}
	break;

	case T_RMID:{iError=1;;}break; 
	case T_SMF:{iError=1;;}break;
	case T_XMF:{iError=1;;}break;
	case T_SNG:{iError=1;;}break;
	case T_MUS:{iError=1;;}break;
	
	default:{
	   iError=1;
	   if(iRet==-1){
	      /* not MIDI file, do nothing */
	      amTrace((const U8*)"It's not valid MIDI file...\n");
	      
	      fprintf(stderr, "It's not valid MIDI file...\n");
	   } else if(iRet==-2){
	    /* unsupported MIDI type format, do nothing*/
	      
	      amTrace((const U8*)"Unsupported MIDI file format...\n");
	      
	      
	      fprintf(stderr, "Unsupported MIDI file format...\n");
	  }
	  else{
	    /* unknown error, do nothing */
	    amTrace((const U8*)"Unknown error.\n");
	    
	    fprintf(stderr, "Unknown error ...\n");
	    
	  }
	}
	break;
     }
     
      /* free up buffer, we don't need it anymore */
      if(pMidi!=NULL)
	Mfree(pMidi);pMidi=NULL;
	  
	  
    if(iError!=1){
     /* play preloaded tune */
        playMidi(&midiTune);
    }
     
    } /* MIDI loading failed */
    else{
      amTrace((const U8*)"Error: Couldn't read file...\n");
      
      fprintf(stderr, "Error: Couldn't read file...\n");
      return(-1);
    }

    /* turn off all notes on channels 0-15 */
    am_allNotesOff(16);
   
    /* clean up, free internal library buffers etc..*/
    am_deinit();
   
//     /* just the test TA and TB */ 
//    installTickCounter();
//    installTA();
//     /*interrupt handler test */
//     for(;;){
//       printf("tb: %u, ta: %x \n",(unsigned int)counter,(unsigned int)cA);
//     }
//       deinstallTickCounter();
 return (0);
}

/* variable quantity reading test */
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
     	amTrace((const U8*)"VLQ value:%x, decoded: %x, size: %d\n",(unsigned int)val[iCounter], (unsigned int)result, valsize );
	
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
  
  amTrace((const U8*)"\nplayMidi: time division: %d[MPQ], track tempo:%u [ms], tick: %4.3f\n",td,(unsigned int)trackTempo,tick);
  
  
  //print sequence info 
  printf("Now playing:\n");
  printf("Sequence name: %s\n",pMidiSequence->pSequenceName);
  
  //retrieve track list
  printf("Tracks: %s\n",pMidiSequence->pSequenceName);
  
  /* get first event */
  pMyEvent= &((pMidiSequence->arTracks[0])->trkEventList); 
  /* play our sequence - send all events  */		      
  /*printEventList( &pMyEvent );*/
  
  /* deltas are relative to the last event */
  amTrace((const U8*)"Sending all events with delta: %u\n", (unsigned int)currDelta);
  
  while((currDelta=sendMidiEvents(lastDelta, &pMyEvent))){
    amTrace((const U8*)"Sending all events with delta: %u\n", (unsigned int)currDelta);
    
    lastDelta=currDelta;
  }
  
  amTrace((const U8*)"File processed successfully. ");
  
  
}
