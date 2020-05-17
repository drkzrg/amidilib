
/**  Copyright 2007-2020 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>

//#define MANUAL_STEP 1

#include "amidilib.h"
#include "amidiseq.h"       // sequence structs
#include "fmio.h"           // disc i/o
#include "timing/mfp.h"
#include "timing/miditim.h"

// nkt conversion
#include "nkt.h"
#include "seq2nkt.h"

#ifdef MIDI_PARSER_TEST
#include "containers/list.h"
#endif

#include "input/ikbd.h"

#ifdef MANUAL_STEP
extern void updateStepSingle(void);
extern void updateStepMulti(void);
#endif

// display info screen
void printInfoScreen(void); 
void displayTuneInfo(void);
void mainLoop(sSequence_t *pSequence, const char *pFileName);

#ifdef MIDI_PARSER_TEST
void midiParserTest(sSequence_t *pSequence);
#endif

/**
 * main program entry
 */

int main(int argc, char *argv[])
{
  
    uint16 iRet = 0;
    int16 iError = 0;
    
    /* init library */
    iError = amInit();
    
    if( ((argc>=1) && strlen(argv[1])!=0)){
      printf("Trying to load %s\n",argv[1]);
    }else{
      printf("No specified midi filename! exiting\n");
      amDeinit();
      return 0;
    }
    
    // load midi file into memory 
    uint32 ulFileLenght = 0L;
    void *pMidiData = loadFile((uint8 *)argv[1], PREFER_TT, &ulFileLenght);

    if(pMidiData!=NULL)
    {
     printf("Midi file loaded, size: %u bytes.\n",(unsigned int)ulFileLenght);
     
     /* process MIDI*/
      printf("Please wait...\n");

      sSequence_t *pMusicSeq=0; //here we store our sequence data
      iError = amLoadMidiFile(argv[1], pMidiData, &pMusicSeq);

      /* free up buffer with loaded midi file, we don't need it anymore */
      amFree(pMidiData);

      if(iError==0)
      {

	     printf("Sequence name: %s\n",pMusicSeq->pSequenceName);
	     printf("Nb of tracks: %d\n",pMusicSeq->ubNumTracks);
       printf("PPQN: %u\n",pMusicSeq->timeDivision);
	  
	     #ifdef MIDI_PARSER_TEST
        //output loaded midi file to screen/log
        midiParserTest(pMusicSeq);
	     #endif

	     printInfoScreen();    
       mainLoop(pMusicSeq,argv[1]);
	  
	     //unload sequence
	     amDestroySequence(&pMusicSeq);
	  
      } else {

        amTrace((const uint8*)"Error while parsing. Exiting... \n");
    
        //unload sequence
        amDestroySequence(&pMusicSeq);
        amDeinit(); //deinit our stuff
        return(-1);

      }
     
    } else { /* MIDI loading failed */
      amTrace((const uint8*)"Error: Couldn't read %s file...\n",argv[1]);
      printf( "Error: Couldn't read %s file...\n",argv[1]);
      amDeinit();	//deinit our stuff
      return(-1);
    }

 deinstallReplayRout();   
 amDeinit();
 return (0);
}


void mainLoop(sSequence_t *pSequence, const char *pFileName)
{
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
	  bool bQuit=FALSE;
	  
	  //####
      while(bQuit==FALSE){

	    //check keyboard input  
	    for (uint16 i=0; i<128; ++i) {
	    
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

        case SC_D:{
          //dumps loaded sequence to NKT file
          {
            printf("Convert sequence to NKT format...\n");
            char tempName[128]={0};
            char *pTempPtr=0;
            sNktSeq *pNktSeq=0;

            //set midi output name
             strncpy(tempName,pFileName,strlen(pFileName));
             pTempPtr=strrchr(tempName,'.');
             memcpy(pTempPtr+1,"NKT",4);

            if(Seq2NktFile(pSequence, tempName, FALSE)<0){
               printf("Error during NKT format conversion..\n");
            }else{
                printf("File saved: %s.\n",tempName);
            }

          }
        } break;
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

                if(pSequence->seqType==ST_SINGLE){
                    updateStepSingle();
                }
                else if(pSequence->seqType==ST_MULTI){
                    updateStepMulti();
                }else if(pSequence->seqType==ST_MULTI_SUB){
                    printf("!!! ST_MULTI_SUB update TODO...\n");
                }
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


void printInfoScreen(void){
  
  const sAMIDI_version *pInfo=amGetVersionInfo();
  
  printf("\n=========================================\n");
  printf(LIB_NAME);
  printf("v.%d.%d.%d\t",pInfo->major,pInfo->minor,pInfo->patch);
  printf("date: %s %s\n",__DATE__,__TIME__);
  
  printf("    [i] - display tune info\n");
  printf("    [p] - play loaded tune\n");
  printf("    [r] - pause/unpause played sequence \n");
  printf("    [m] - toggle play once / loop mode \n");
  printf("    [d] - Convert sequence to NKT format.\n");
  printf("    [h] - show this help screen \n");
  printf("\n    [spacebar] - stop sequence replay \n");
  printf("    [Esc] - quit\n");
  printf(AMIDI_INFO);
  printf("==========================================\n");
  printf("Ready...\n");
} 

void displayTuneInfo(void){

  sSequence_t *pPtr;
  getCurrentSeq(&pPtr);
  
  uint32 tempo=pPtr->arTracks[0]->currentState.currentTempo;
  uint16 td=pPtr->timeDivision;
  uint16 numTrk=pPtr->ubNumTracks;
  
  printf("Sequence name %s\n",pPtr->pSequenceName);
  printf("PPQN: %u\t",td);
  printf("Tempo: %lu [ms]\n",tempo);
  
  printf("Number of tracks: %d\n",numTrk);
  uint8 *pTrkName=0;
  for(int i=0;i<numTrk;i++){
    pTrkName=pPtr->arTracks[i]->pTrackName;
    printf("[Track no. %d] %s\n",(i+1),pTrkName);
  }
  
  printf("\nReady...\n");
}


#ifdef MIDI_PARSER_TEST
void midiParserTest(sSequence_t *pSequence){
   amTrace((const uint8*)"Parsed MIDI read test\n");
   amTrace((const uint8*)"Sequence name: %s\n",pSequence->pSequenceName);
   amTrace((const uint8*)"Nb of tracks: %d\n",pSequence->ubNumTracks);
   amTrace((const uint8*)"Td/PPQN: %u\n",pSequence->timeDivision);
   amTrace((const uint8*)"Active track: %d\n",pSequence->ubActiveTrack);
	  
	  //output data loaded in each track
  for (uint16 i=0;i<pSequence->ubNumTracks;++i){
    sTrack_t *p=pSequence->arTracks[i];
    
    if(p!=0){
	amTrace((const uint8*)"Track #[%d] \t",i+1);
	amTrace((const uint8*)"Track name: %s\n",p->pTrackName);
	amTrace((const uint8*)"Track ptr %p\n",p->pTrkEventList);
	
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

