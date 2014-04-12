

#include "nkt.h"
#include "amidilib.h"

#include "input/ikbd.h"
#include "timing/mfp.h"
#include "timing/miditim.h"
#include "amlog.h"

//#define MANUAL_STEP 0

#ifdef MANUAL_STEP
extern void updateStepNkt();
#endif

void printInfoScreen();

void mainLoop(sNktSeq *pSequence);

int main(int argc, char *argv[]){
sNktSeq *pNktSeq=0;
 S16 iError=0;

   if(argc>=1&&argv[1]!='\0'){
        fprintf(stderr,"Trying to load %s\n",argv[1]);
    }else{
        fprintf(stderr,"No specified nkt filename! exiting\n");
        am_deinit();
        return 0;
    }

    am_init();

    flushMidiSendBuffer();

    float time=0,delta=0;
    time = getTimeStamp();
    pNktSeq=loadSequence(argv[1]);
    delta=getTimeDelta();

    if(pNktSeq!=NULL){
        fprintf(stderr,"MIDI file parsed in ~%4.2f[sec]/~%4.2f[min]\n", delta, delta / 60.0f);

        printInfoScreen();
        mainLoop(pNktSeq);

        // destroy sequence
        destroySequence(pNktSeq);
        pNktSeq=0;

        deinstallReplayRout();
        am_deinit(); //deinit our stuff

    }else{
        printf("Error: Loading %s failed.\n", argv[1]);
    }


  return 0;
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

  sNktSeq *pPtr=0;
  getCurrentSequence(&pPtr);

  U32 tempo=pPtr->currentTempo;
  U16 td=pPtr->timeDivision;

  printf("PPQN: %u\t",td);
  printf("Tempo: %lu [ms]\n",tempo);

  printf("\nReady...\n");
}

void mainLoop(sNktSeq *pSequence){
    BOOL bQuit=FALSE;
#ifdef MANUAL_STEP
    initSequenceManual(pSequence,NKT_PLAY_ONCE);
#else
    initSequence(pSequence,NKT_PLAY_ONCE);
#endif

    //install replay rout
      amMemSet(Ikbd_keyboard, KEY_UNDEFINED, sizeof(Ikbd_keyboard));
      Ikbd_mousex = Ikbd_mousey = Ikbd_mouseb = Ikbd_joystick = 0;

      /* Install our asm ikbd handler */
      Supexec(IkbdInstall);

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
          stopSequence();
        }break;
        case SC_P:{
          //starts playing sequence if is stopped
          playSequence();
         }break;
        case SC_R:{
          //pauses sequence when is playing
           pauseSequence();
         }break;
         case SC_M:{
          //toggles between play once and play in loop
          switchReplayMode();
         }break;
        case SC_I:{
           //displays current track info
           displayTuneInfo();
         }break;
        case SC_H:{
           //displays help/startup screen
          printInfoScreen();
         }break;
#ifdef MANUAL_STEP
          case SC_ENTER:{

            for(int i=0;i<SEQUENCER_UPDATE_HZ;++i){
                updateStepNkt();
            }

            printNktSequenceState();

            // clear buffer after each update step
            flushMidiSendBuffer();

          }break;
#endif
         case SC_SPACEBAR:{
            stopSequence();
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
}
