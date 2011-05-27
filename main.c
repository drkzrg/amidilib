
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
#include "include/ikbd.h"
#include "include/scancode.h"

static const U8 KEY_PRESSED = 0xff;
static const U8 KEY_UNDEFINED=0x80;
static const U8 KEY_RELEASED=0x00;

#endif

/**
 * main program entry
 */
 
volatile sSequence_t *currentState;	

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
    sSequence_t *pMidiTune=0;	//here we store our sequence data
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

	iError=am_handleMIDIfile(pMidi, ulFileLenght,&pMidiTune);
  
	delta=getTimeDelta();
	 
	/* free up buffer with loaded midi file, we don't need it anymore */
	 amFree(&pMidi);
	 
	 if(iError==0){
	  printf("MIDI file parsed in ~%4.2f[sec]/~%4.2f[min]\n",delta,delta/60.0f);
	  amTrace((const U8*)"MIDI file parsed in ~%4.2f[sec]/~%4.2f[min]\n",delta,delta/60.0f);
	
	  #ifdef MIDI_PARSER_TEST
	  //output loaded midi file 
	  amTrace((const U8*)"Parsed MIDI read test\n");
	  amTrace((const U8*)"Sequence name: %s\n",pMidiTune->pSequenceName);
	  amTrace((const U8*)"Nb of tracks: %d\n",pMidiTune->ubNumTracks);
	  amTrace((const U8*)"PPQN: %d\n",pMidiTune->timeDivision);
	  amTrace((const U8*)"Active track: %d\n",pMidiTune->ubActiveTrack);
	  
	  //output data loaded in each track
	  for (int i=0;i<pMidiTune->ubNumTracks;i++){
	    sTrack_t *p=pMidiTune->arTracks[i];
	    
	    if(p!=0){
		amTrace((const U8*)"Track #[%d] \t",i+1);
		amTrace((const U8*)"Track name: %s\n",p->pInstrumentName);
		amTrace((const U8*)"Track ptr %p\n",p->pTrkEventList);
	  
		//print out all events
		if(p->pTrkEventList!=0){
		  sEventList *pEventList=p->pTrkEventList;
		
		  while(pEventList!=0){
		    printEventBlock(&(pEventList->eventBlock));
		    pEventList=pEventList->pNext;
		  }
		}
		////
	    }
	  }
	  #endif
	   printf("Ready...\n");
#ifndef PORTABLE
	  /* Install our asm ikbd handler */
	  Supexec(IkbdInstall);
	  amMemSet(Ikbd_keyboard, KEY_UNDEFINED, sizeof(Ikbd_keyboard));
	  Ikbd_mousex = Ikbd_mousey = Ikbd_mouseb = Ikbd_joystick = 0;
	  BOOL bQuit=FALSE;
	  BOOL tick=TRUE; //when we start we have to push all the events with delta 0
			  
	  U32 tickCounter=0;
	  
	    while(bQuit!=TRUE){
	    //battle plan:
	    //check internal counter
	    //for each track maintain internal counter
	    //if (internal counter == current event delta)
	    //	 while (internal counter == current event delta)
	    // 	 {send an event} reset internal track counter
	    // else internal counter++; 
	    
	    if(tick==TRUE){
	      printf("Tick! Counter: %d\n",tickCounter);
	    }
	    
	    tick=FALSE; //reset internal manual tick indicator ;>
	      
	    for (int i=0; i<128; i++) {
	    if (Ikbd_keyboard[i]==KEY_PRESSED) {
	      Ikbd_keyboard[i]=KEY_UNDEFINED;
	      
	      switch(i){
		case SC_ESC:{
		  bQuit=TRUE;
		  printf("Quiting\n");
		}break;
		case SC_A:{
		 tick=TRUE;
		  //printf("Counter: %d\n",tickCounter);
		 tickCounter++;
		}break;
		case SC_SPACEBAR:{
		  printf("Stop. Reset counter.\n");
		  am_allNotesOff(16);
		}break;
	      };
	      //end of switch
	    }
	  }	  
	 }
	  /* Uninstall our ikbd handler */
	  Supexec(IkbdUninstall);
#else
#warning Portable main loop unimplemented
#endif
	   //unload sequence
	   am_destroySequence(&pMidiTune);
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

