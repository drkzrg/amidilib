
/**  Copyright 2007-2010 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>

#include "include/amidilib.h"
#include "include/list/list.h"
#include "include/mfp.h"

/**
 * main program entry
 */


typedef struct{
  volatile U32 currentTempo;		// quaternote duration in ms, 500ms default
  volatile U32 currentPPQN;		// pulses per quater note
  volatile U32 currentIdx;		// current position in table
  volatile sSequence_t *seqPtr[16];	// sequence ptr array, max 16 tracks, we don't need more
  volatile U32 activeSequence;		// default 0
  U32 numSequences;			// number of tracks in total (only valid for MIDI file type 2 or other with multiple,
					// independent tracks)
  
  volatile U32 state;			// 0=STOP, 1-PLAYING, 2-PAUSED
} sCurrentSequenceState;

enum{
 STOP=0, 
 PLAY_ONCE=1, 
 PLAY_LOOP=2,
 PLAY_RANDOM=3,
 PAUSED=4
} eSeqState;

void playSeq (sSequence_t *seq,U32 mode,U32 data, volatile sCurrentSequenceState *pInitialState);
void VLQtest(void);
void memoryCheck(void);


extern volatile U8 tbData,tbMode;
volatile sCurrentSequenceState currentState;
volatile extern U32 counter;
extern U32 defaultPlayMode;



int main(int argc, char *argv[]){
    U32 defaultTempo=60000000/120;
    U8 currentChannel=1;
    U8 currentVelocity=127;
    U8 currentPN=127;
    U8 currentBankSelect=0;
  
    void *pMidi=NULL;
    U16 iRet=0;
    S16 iError=0;
    
    currentState.currentTempo=defaultTempo;
    currentState.currentPPQN=120;
  
    
    /* init library */
    iError=am_init();
    
    //set current channel as 1, default is 0 in external module
    control_change(0x00, currentChannel, currentBankSelect,0x00);
    program_change(currentChannel, currentPN);
  
    
    //trace is set up in am_init()
    
    VLQtest();
    memoryCheck();
    
    if(argc>=1&&argv[1]!='\0'){
      amTrace("Trying to load %s\n",argv[1]);
    }
    else{
      amTrace("No specified midi filename! exiting\n");
      am_deinit();
      return 0;
    }
       
    /* get connected device info */
    getConnectedDeviceInfo();
    
    U32 ulFileLenght=0L;
    pMidi=loadFile((U8 *)argv[1], PREFER_TT, &ulFileLenght);

    sSequence_t midiTune;	//here we store our sequence data
    
    if(pMidi!=NULL){

     amTrace((const U8*)"Midi file loaded, size: %u bytes.\n",(unsigned int)ulFileLenght);
     
     /* process MIDI*/
     /* check midi header */
     double ms;
     
     printf("Please wait...\n");
     
     clock_t begin=clock();
     iError=am_handleMIDIfile(pMidi, ulFileLenght,&midiTune);
     clock_t end=clock();
	  
      ms=am_diffclock(end,begin);

      if(iError==0){
	printf("MIDI file parsed in ~%4.2f[sec]/~%4.2f[min](%6.4f [ms])\n",ms/1000.0f,ms/1000.0f/60.0f,ms);
	amTrace((const U8*)"MIDI file parsed in ~%4.2f[sec]/~%4.2f[min](%6.4f [ms])\n",ms/1000.0f,ms/1000.0f/60.0f,ms);
      }else{
	amTrace((const U8*)"Error while parsing. Exiting... \n");
      }
     
      /* free up buffer with loaded midi file, we don't need it anymore */

      if(pMidi!=NULL){
	Mfree(pMidi);
	pMidi=NULL;
      }
	  
	  
    if(iError==0){
     /* play preloaded tune if no error occured */
        playSeq(&midiTune,1,1, &currentState);
      /* loop and wait for keypress */
	BOOL bQuit=FALSE;
	printf("Press q to quit...\n");
    
      while(bQuit!=TRUE){
	//char c=getchar(); if(c=='q'||c=='Q') bQuit=TRUE;
	printf("counter %u\n",(unsigned int)counter);
      }
  
	deinstallMIDIreplay();
    }
    
     
    } /* MIDI loading failed */
    else{
      amTrace((const U8*)"Error: Couldn't read %s file...\n",argv[1]);
      fprintf(stderr, "Error: Couldn't read %s file...\n",argv[1]);
      return(-1);
    }

    /* turn off all notes on channels 0-15 */
    am_allNotesOff(16);
   
    /* clean up, free internal library buffers etc..*/
    am_deinit();

 return (0);
}

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


void memoryCheck(void){
    U32 mem=0;
    amTrace((const U8*)"System memory check:\n");
	
    /* mem tst */
    mem=getFreeMem(ST_RAM);
    amTrace((const U8*)"ST-RAM: %u\n",(U32)mem);
	
    mem=getFreeMem(TT_RAM);
    amTrace((const U8*)"TT-RAM: %u\n",(U32)mem);
	
    mem=getFreeMem(PREFER_ST);
    amTrace((const U8*)"Prefered ST-RAM: %u\n",(U32)mem);
	
    mem=getFreeMem(PREFER_TT);
    amTrace((const U8*)"Prefered TT-RAM: %u\n",(U32)mem);
	
}

extern volatile sEventItem *g_pEventPtr;

void playSeq(sSequence_t *seq, U32 mode,U32 data, volatile sCurrentSequenceState *pInitialState){

    pInitialState->currentIdx=0;			//initial position
    pInitialState->state=STOP;				//track state
    pInitialState->seqPtr[0]=seq;			//ptr to sequence

  //install replay routine 96 ticks per 500ms interval

  return;
}
