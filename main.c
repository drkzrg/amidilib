
/**  Copyright 2007-2010 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#include "include/amidilib.h"
#include "include/amlog.h"
#include "include/list/list.h"


#ifndef PORTABLE
#include "include/mfp.h"
#endif

/**
 * main program entry
 */
 
volatile sSequence_t *currentState;	

enum{
 STOP=0, 
 PLAY_ONCE=1, 
 PLAY_LOOP=2,
 PLAY_RANDOM=3,
 PAUSED=4
} eSeqState;

void VLQtest(void);
void memoryCheck(void);

void playSeq (sSequence_t *seq);

#ifdef PORTABLE
void deinstallReplayRout(void);
void turnOffKeyclick(void);
U32 defaultPlayMode;
#else
extern volatile U8 tbData,tbMode;
volatile extern U32 counter;
extern void installReplayRout(U8 mode,U8 data,volatile sSequence_t *pPtr);
extern void deinstallReplayRout();
extern void turnOffKeyclick(void);
extern U32 defaultPlayMode;
#endif

#ifdef _MSC_VER
#include "stdafx.h";
int _tmain(int argc, _TCHAR* argv[]){
#else
int main(int argc, char *argv[]){
#endif
    sSequence_t midiTune;	//here we store our sequence data
    U8 currentChannel=1;
    U8 currentVelocity=127;
    U8 currentPN=127;
    U8 currentBankSelect=0;
    U32 ulFileLenght=0L;
    float time=0;
    float delta=0;
    BOOL bQuit=0;

    void *pMidi=NULL;
    U16 iRet=0;
    S16 iError=0;

    /* init library */
    iError=am_init();
    
    //set current channel as 1, default is 0 in external module
    control_change(0x00, currentChannel, currentBankSelect,0x00);
    program_change(currentChannel, currentPN);
    
    if(argc>=1&&argv[1]!='\0'){
      amTrace(( U8 *)"Trying to load %s\n",argv[1]);
    }
    else{
      amTrace(( U8 *)"No specified midi filename! exiting\n");
      am_deinit();
      return 0;
    }
    
    pMidi=loadFile((U8 *)argv[1], PREFER_TT, &ulFileLenght);

    if(pMidi!=NULL){

     amTrace((const U8*)"Midi file loaded, size: %u bytes.\n",(unsigned int)ulFileLenght);
     
     /* process MIDI*/
     /* check midi header */
	 printf("Please wait...\n");
	 time = getTimeStamp();

	iError=am_handleMIDIfile(pMidi, ulFileLenght,&midiTune);
  
	delta=getTimeDelta();
	 
	 if(iError==0){
	   printf("MIDI file parsed in ~%4.2f[sec]/~%4.2f[min]\n",delta,delta/60.0f);
	   amTrace((const U8*)"MIDI file parsed in ~%4.2f[sec]/~%4.2f[min]\n",delta,delta/60.0f);
	   
	   /* free up buffer with loaded midi file, we don't need it anymore */
	   amFree(pMidi);
	   pMidi=NULL;
		
	   // MAIN LOOP 
		/* play preloaded tune if no error occured */
		//playSeq(&midiTune);
		
		/* loop and wait for keypress */
		bQuit=FALSE;
		printf("Press q to quit...\n");
    
		while(bQuit!=TRUE){
		 //char c=getchar(); if(c=='q'||c=='Q') bQuit=TRUE;
		 //printf("counter %u\n",(unsigned int)counter);
		}
		
		
	  //END of MAINLOOP	
      }else{
		amTrace((const U8*)"Error while parsing. Exiting... \n");
		am_deinit(); //deinit our stuff
        return(-1);
      }
     
    } /* MIDI loading failed */
    else{
      amTrace((const U8*)"Error: Couldn't read %s file...\n",argv[1]);
      fprintf(stderr, "Error: Couldn't read %s file...\n",argv[1]);
	  am_deinit();	//deinit our stuff
      return(-1);
    }

      am_allNotesOff(16);
      deinstallReplayRout();   
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

void playSeq(sSequence_t *seq){
#ifdef PORTABLE
	//TODO! 
#else
  U32 freq=seq->arTracks[0]->currentState.currentTempo/seq->timeDivision;
  U32 mode,data;

  getMFPTimerSettings(freq,&mode,&data);
  installReplayRout(mode, data, seq);

#endif
 return;
}

#ifdef PORTABLE
void installReplayRout(U8 mode,U8 data,volatile sSequence_t *pPtr){
//todo  
#warning installReplayRout() unimplemented! 
}


void deinstallReplayRout(void){
//TODO!
#warning deinstallReplayRout() unimplemented! 
}

void turnOffKeyclick(void){
  //todo
#warning turnOffKeyclick() unimplemented!
}

#endif

