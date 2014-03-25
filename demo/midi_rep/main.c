
/**  Copyright 2007-2010 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#include "amidilib.h"
#include "amidiseq.h"	//sequence structs
#include "fmio.h"	//disc i/o
#include "timing/miditim.h" 

#ifdef MIDI_PARSER_TEST
#include "list/list.h"
#endif

#include "input/ikbd.h"

//#define MANUAL_STEP 1

#ifdef MANUAL_STEP
extern void updateStep();
#endif

// display info screen
void printInfoScreen(); 
void displayTuneInfo();
void mainLoop(sSequence_t *pSequence);

#ifdef MIDI_PARSER_TEST
void midiParserTest(sSequence_t *pSequence);
#endif
/**
 * main program entry
 */

int main(int argc, char *argv[]){
  
    void *pMidi=NULL;
    U16 iRet=0;
    S16 iError=0;
    sSequence_t *pMidiTune=0;	//here we store our sequence data
    
    /* init library */
    iError=am_init();
    
    if(argc>=1&&argv[1]!='\0'){
      fprintf(stderr,"Trying to load %s\n",argv[1]);
    }else{
      fprintf(stderr,"No specified midi filename! exiting\n");
      am_deinit();
      return 0;
    }
    
    // load midi file into memory 
    U32 ulFileLenght=0L;
    pMidi=loadFile((U8 *)argv[1], PREFER_TT, &ulFileLenght);

    if(pMidi!=NULL){
     fprintf(stderr,"Midi file loaded, size: %u bytes.\n",(unsigned int)ulFileLenght);
     
     /* process MIDI*/
     /* check midi header */
      fprintf(stderr,"Please wait...\n");

      float time=0,delta=0;
      time = getTimeStamp();
      iError=am_handleMIDIfile(argv[1],pMidi, ulFileLenght,&pMidiTune);
      delta=getTimeDelta();
      
      /* free up buffer with loaded midi file, we don't need it anymore */
      amFree(&pMidi);

      if(iError==0){
	  fprintf(stderr,"MIDI file parsed in ~%4.2f[sec]/~%4.2f[min]\n",delta,delta/60.0f);
	
	  fprintf(stderr,"Sequence name: %s\n",pMidiTune->pSequenceName);
	  fprintf(stderr,"Nb of tracks: %d\n",pMidiTune->ubNumTracks);
      fprintf(stderr,"PPQN: %u\n",pMidiTune->timeDivision);
	  
	  #ifdef MIDI_PARSER_TEST
	  //output loaded midi file to screen/log 
	  midiParserTest(pMidiTune);
	  #endif
	  
	  printInfoScreen();    
	  mainLoop(pMidiTune);
	  
	  //unload sequence
	  am_destroySequence(&pMidiTune);
	  //END of MAINLOOP	
      }else{
        amTrace((const U8*)"Error while parsing. Exiting... \n");
        //unload sequence
        am_destroySequence(&pMidiTune);
        am_deinit(); //deinit our stuff
        return(-1);
      }
     
    }else{ /* MIDI loading failed */
      amTrace((const U8*)"Error: Couldn't read %s file...\n",argv[1]);
      fprintf(stderr, "Error: Couldn't read %s file...\n",argv[1]);
      am_deinit();	//deinit our stuff
      return(-1);
    }

 deinstallReplayRout();   
 am_deinit();
 return (0);
}


void mainLoop(sSequence_t *pSequence){
      //install replay rout
#ifdef MANUAL_STEP
    initSeqManual(pSequence);
#else
    initSeq(pSequence,MFP_TiC);
#endif

	  amMemSet(Ikbd_keyboard, KEY_UNDEFINED, sizeof(Ikbd_keyboard));
	  Ikbd_mousex = Ikbd_mousey = Ikbd_mouseb = Ikbd_joystick = 0;
	   
	  /* Install our asm ikbd handler */
	  Supexec(IkbdInstall);
	  BOOL bQuit=FALSE;
	  
	  //####
      while(bQuit==FALSE){

	    //check keyboard input  
	    for (int i=0; i<128; i++) {
	    
	      if (Ikbd_keyboard[i]==KEY_PRESSED) {
	      Ikbd_keyboard[i]=KEY_UNDEFINED;
	      
	      switch(i){
		case SC_ESC:{
		  bQuit=TRUE;
		  //stop sequence
		  stopSeq();
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
		case SC_I:{
		   //displays current track info
		  displayTuneInfo();
		 }break;
		case SC_H:{
		   //displays help/startup screen
		  printInfoScreen();
		 }break;

		 case SC_SPACEBAR:{
		  stopSeq();
		 }break;
          #ifdef MANUAL_STEP

          case SC_ENTER:{

            for(int i=0;i<SEQUENCER_UPDATE_HZ;++i){
                updateStep();
            }

            printSequenceState();

            // clear buffer after each update step
            MIDIbytesToSend=0;
            amMemSet(MIDIsendBuffer,0,32*1024);

          }break;
          #endif
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
}


void printInfoScreen(){
  
  const sAMIDI_version *pInfo=am_getVersionInfo();
  
  printf("\n=========================================\n");
  printf(LIB_NAME);
  printf("v.%d.%d.%d\t",pInfo->major,pInfo->minor,pInfo->patch);
  printf("date: %s %s\n",__DATE__,__TIME__);
  
  printf("    [p] - play loaded tune\n");
  printf("    [r] - pause/unpause played sequence \n");
  printf("    [m] - toggle play once/loop mode\n");
  printf("    [i] - display tune info\n");  
  printf("    [h] - show this help screen\n");  
  printf("\n    [spacebar] - stop sequence replay \n");
  printf("    [Esc] - quit\n");
  printf(AMIDI_INFO);
  printf("==========================================\n");
  printf("Ready...\n");
} 

void displayTuneInfo(){

  sSequence_t *pPtr;
  getCurrentSeq(&pPtr);
  
  U32 tempo=pPtr->arTracks[0]->currentState.currentTempo;
  U16 td=pPtr->timeDivision;
  U16 numTrk=pPtr->ubNumTracks;
  
  printf("Sequence name %s\n",pPtr->pSequenceName);
  printf("PPQN: %u\t",td);
  printf("Tempo: %lu [ms]\n",tempo);
  
  printf("Number of tracks: %d\n",numTrk);
  U8 *pTrkName=0;
  for(int i=0;i<numTrk;i++){
    pTrkName=pPtr->arTracks[i]->pTrackName;
    printf("[Track no. %d] %s\n",(i+1),pTrkName);
  }
  
  printf("\nReady...\n");
}


#ifdef MIDI_PARSER_TEST
void midiParserTest(sSequence_t *pSequence){
   amTrace((const U8*)"Parsed MIDI read test\n");
   amTrace((const U8*)"Sequence name: %s\n",pSequence->pSequenceName);
   amTrace((const U8*)"Nb of tracks: %d\n",pSequence->ubNumTracks);
   amTrace((const U8*)"Td/PPQN: %u\n",pSequence->timeDivision);
   amTrace((const U8*)"Active track: %d\n",pSequence->ubActiveTrack);
	  
	  //output data loaded in each track
  for (int i=0;i<pSequence->ubNumTracks;i++){
    sTrack_t *p=pSequence->arTracks[i];
    
    if(p!=0){
	amTrace((const U8*)"Track #[%d] \t",i+1);
	amTrace((const U8*)"Track name: %s\n",p->pTrackName);
	amTrace((const U8*)"Track ptr %p\n",p->pTrkEventList);
	
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
}
#endif

