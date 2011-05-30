
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

#ifndef PORTABLE
#include "include/ikbd.h"
#include "include/scancode.h"
#include <lzoconf.h>
static const U8 KEY_PRESSED = 0xff;
static const U8 KEY_UNDEFINED=0x80;
static const U8 KEY_RELEASED=0x00;
extern void turnOffKeyclick(void);
#else
void turnOffKeyclick(void);
#endif

// display info screen
void printInfoScreen(); 

/**
 * main program entry
 */

#ifdef _MSC_VER
#include "stdafx.h";
#include <lzoconf.h>
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
      printf("Trying to load %s\n",argv[1]);
    }
    else{
      printf("No specified midi filename! exiting\n");
      am_deinit();
      return 0;
    }
    
    pMidi=loadFile((U8 *)argv[1], PREFER_TT, &ulFileLenght);

    if(pMidi!=NULL){

     printf("Midi file loaded, size: %u bytes.\n",(unsigned int)ulFileLenght);
     
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
	  
		//log state
		amTrace((const U8*)"state: \n");
		amTrace((const U8*)"internalCounter: %d \n",p->currentState.deltaCounter);
		amTrace((const U8*)"Start event ptr: %p \n",p->currentState.pStart);
		amTrace((const U8*)"Current event ptr: %p \n",p->currentState.pCurrent);
		
		//print out all events
		if(p->pTrkEventList!=0){
		  sEventList *pEventList=p->pTrkEventList;
		
		  while(pEventList!=0){
		    printEventBlock(&(pEventList->eventBlock));
		    pEventList=pEventList->pNext;
		  }
		}
	    }
	  }
	  #endif
	  printInfoScreen();
    
	  printf("Ready...\n");
#ifndef PORTABLE
	  amMemSet(Ikbd_keyboard, KEY_UNDEFINED, sizeof(Ikbd_keyboard));
	  Ikbd_mousex = Ikbd_mousey = Ikbd_mouseb = Ikbd_joystick = 0;
	   /* Install our asm ikbd handler */
	  Supexec(IkbdInstall);
	  BOOL bQuit=FALSE;

	  //install replay rout and play tune
	  initSeq(pMidiTune);
	  
	  while(bQuit!=TRUE){
	    //check keyboard input  
	    for (int i=0; i<128; i++) {
	      
	    if (Ikbd_keyboard[i]==KEY_PRESSED) {
	      Ikbd_keyboard[i]=KEY_UNDEFINED;
	      
	      switch(i){
		case SC_ESC:{
		  bQuit=TRUE;
		  printf("Quiting\n");
		}break;
		case SC_P:{
		  //starts playing sequence if is stopped
		  playSeq();
		 }break;
		case SC_R:{
		  //pauses sequence when is playing
		   pauseSeq();
		 }break;
		 case SC_M:{
		   //toggles between play once and play in loop
		  toggleReplayMode();
		 }break;
		case SC_SPACEBAR:{
		  stopSeq();
		 }break;
	      };
	      //end of switch
	    }
	    if (Ikbd_keyboard[i]==KEY_RELEASED) {
	      Ikbd_keyboard[i]=KEY_UNDEFINED;
	    }
	      
	   } //end of for 	  
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

void printInfoScreen(){
  printf("\n=========================================\n");
  printf("  /|\\ amidi demo..\n");
  printf("  MIDI type 0,1 replay\n\n");  
  printf("    [p] - play loaded tune\n");
  printf("    [r] - pause/unpause played sequence \n");
  printf("    [spacebar] - stop sequence replay \n");
  printf("    [Esc] - quit\n");
  printf("\n(c) nokturnal 05'2011\n");
  printf("\nwww: http://nokturnal.pl\n");
  printf("==========================================\n");
} 


#ifdef PORTABLE
void turnOffKeyclick(void){
//todo
#warning turnOffKeyclick() unimplemented!
}

#endif

