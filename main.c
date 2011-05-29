
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
 
#ifdef PORTABLE
void deinstallReplayRout(void);
void turnOffKeyclick(void);
U32 defaultPlayMode;
#else


////// TODO: move it to separate file
static volatile U8 tbData,tbMode;
static volatile U32 defaultPlayMode;
static volatile U32 deltaCounter;
static volatile sSequence_t *pCurrentSequence=0;		//here is stored current sequence

// install and deinstall routines
extern void installReplayRout(U8 mode,U8 data);
extern void deinstallReplayRout();

// update sequence function called from exception routine
void sequenceUpdate(void);

//sequence control interface
BOOL isSeqPlaying(void);
void stopSeq(void);
void pauseSeq(void);
void playSeq(void);
void initSeq(sSequence_t *seq);
//////////////////////////////////////////////////////////////////

extern void turnOffKeyclick(void);
#endif

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
		    pauseSeq();
		 }break;
		case SC_SPACEBAR:{
		  stopSeq();
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

void initSeq(sSequence_t *seq){
#ifdef PORTABLE
  //TODO! 
#else
  U32 freq=seq->arTracks[0]->currentState.currentTempo/seq->timeDivision;
  U32 mode=0,data=0;
  
  pCurrentSequence=seq;
  
  getMFPTimerSettings(freq/8,&mode,&data);
  installReplayRout(mode, data);
#endif
 return;
}


//this will be called from an interrupt in each delta increment
void sequenceUpdate(void){
 static sTrack_t *pTrk=0;
 static sEventList *pCurrent=0;
 static evntFuncPtr myFunc=0; 
 BOOL bNoteOffSent=FALSE;
 static U32 silenceThrCounter=0;
 
 if(pCurrentSequence){ 
   //TODO: change it to assert and include only in DEBUG build

   if(pCurrentSequence->arTracks[0]->currentState.playState==PS_PLAYING){
     bNoteOffSent=FALSE; //for handling PS_STOPPED and PS_PAUSED states
    
    //for each track (0-> (numTracks-1) ) 
    for (U32 i=0;i<pCurrentSequence->ubNumTracks;i++){
      pTrk=pCurrentSequence->arTracks[i];

      if(pTrk!=0){
	#ifdef DEBUG_BUILD 
	  amTrace((const U8 *)"************** Handling track: #%d, step: [%u]\n",i+1,(unsigned int)tickCounter);
	#endif	
	  
	//TODO: check if all current pointers aren't null
	//if yes stop replay or loop
	//TODO: adjust handling for multiple, independent tracks
	//TODO: [optional] slap all the pointers to an array and send everything in one go
	//TODO: rewrite this in m68k :P for speeeeed =) so cooooool....
	pCurrent=pTrk->currentState.pCurrent;
		
	//sometimes eventlist will be empty, because there will be nothing interesting
	//to send in parsed file, usually in the first track of MIDI 1 file (for example) 
	if(pCurrent!=NULL){ 
	
	//if (internal counter == current event delta)
	if((pCurrent->eventBlock.uiDeltaTime)==pTrk->currentState.deltaCounter){
#ifdef DEBUG_BUILD 
	printEventBlock(&(pCurrent->eventBlock));
#endif		
	if(pTrk->currentState.bMute!=TRUE){
	  //output data only when track is not mute
	  myFunc= pCurrent->eventBlock.infoBlock.func;
	  (*myFunc)((void *)pCurrent->eventBlock.dataPtr);
	}
		    
	pCurrent=pCurrent->pNext;
	pTrk->currentState.pCurrent=pCurrent;
	//check if next event isn't NULL
	//if yes do nothing
	// else check if event delta==0 if yes keep sending events, otherwise there is nothing to do
	while(((pCurrent!=0)&&(pCurrent->eventBlock.uiDeltaTime==0))){
	//send data with delta==0
#ifdef DEBUG_BUILD 
	printEventBlock(&(pCurrent->eventBlock));
#endif		
	if(pTrk->currentState.bMute!=TRUE){
	  //the same as above
	  myFunc= pCurrent->eventBlock.infoBlock.func;
	  (*myFunc)((void *)pCurrent->eventBlock.dataPtr);
	}
	//next
	  pCurrent=pCurrent->pNext;
	  pTrk->currentState.pCurrent=pCurrent;
	}
	// done reset internal track counter
	// pMidiTune->arTracks[i]->currentState.pCurrent should point to event with NULL or
	// event with delta higher than 0
	pTrk->currentState.deltaCounter=0;
#ifdef DEBUG_BUILD 
	amTrace((const U8 *)"reset track: %d counter\n",i);
#endif
	}else{
	// else internal counter++; 
	pCurrentSequence->arTracks[i]->currentState.deltaCounter++;
#ifdef DEBUG_BUILD 
	amTrace((const U8 *)"increase track %d counter\n",i);
#endif
	}
	}
#ifdef DEBUG_BUILD 
else{
	//quite normal
	amTrace((const U8 *)"Nothing to send in this track..\n");}
#endif
}
#ifdef DEBUG_BUILD 
else{ 
  //this should not happen at all
  amTrace((const U8 *)"Error: Track is NULL wtf?..\n");
  
}
#endif 
    
    //check for end of the track 
    // if we play in loop mode: set state to PS_PLAYING,reset track to the beginning
    // else stop replay: set state to PS_STOPPED, reset track to the beginning
    
   deltaCounter++;
     
  }//end of track loop
  }//track is playing
  else if(pCurrentSequence->arTracks[0]->currentState.playState==PS_STOPPED){
      //reset all counters, but only once
      if(bNoteOffSent==FALSE){
	  for (U32 i=0;i<pCurrentSequence->ubNumTracks;i++){
	    pTrk=pCurrentSequence->arTracks[i];
	    pTrk->currentState.pCurrent=pTrk->currentState.pStart;
	  }
	  //turn all notes off on external module
	  bNoteOffSent=TRUE;
	  am_allNotesOff(16);
	//done! 
      }
  }else if(pCurrentSequence->arTracks[0]->currentState.playState==PS_PAUSED){
      if(bNoteOffSent==FALSE){
	  //turn all notes off on external module but only once
	  bNoteOffSent=TRUE;
	  am_allNotesOff(16);
	}
  }
}//pCurrentSequence null check
}//sequenceUpdate() end

//replay control
BOOL isSeqPlaying(void){
if(((pCurrentSequence!=0)&&(pCurrentSequence->arTracks[0]->currentState.playState==PS_PLAYING))) 
  return TRUE;
else 
  return FALSE;
}


void stopSeq(void){
//printf("Stop.\n");
if(((pCurrentSequence!=0)&&(pCurrentSequence->arTracks[0]->currentState.playState!=PS_STOPPED))){
  pCurrentSequence->arTracks[0]->currentState.playState=PS_STOPPED;
}

}

void pauseSeq(){
  sTrack_t *pTrack=0;
  //printf("Pause/Resume.\n");
  if(pCurrentSequence!=0){
    //TODO: handling individual tracks for MIDI 2 type
    // for one sequence(single/multichannel) we will check state of the first track only
    pTrack=pCurrentSequence->arTracks[0];
    switch(pTrack->currentState.playState){
      
      case PS_PLAYING:{
	pTrack->currentState.playState=PS_PAUSED;
      }break;
      case PS_PAUSED:{
	pTrack->currentState.playState=PS_PLAYING;
      }break;
      case PS_STOPPED:{
	pTrack->currentState.playState=PS_PLAYING;
      }break;
      
    };
  }
}

void playSeq(void){
  if(pCurrentSequence!=0){
    //set state
    pCurrentSequence->arTracks[0]->currentState.playState=PS_PLAYING;
  }
  
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

