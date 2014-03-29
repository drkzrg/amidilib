

#include "nkt.h"
#include "amidilib.h"

#include "input/ikbd.h"
#include "timing/mfp.h"

void printInfoScreen();
void mainLoop(sNktSeq *pSequence);



int main(int argc, char *argv[]){
sNktSeq *pNktSeq=0;
 S16 iError=0;

    if(argc>=1&&argv[1]!='\0'){
      fprintf(stderr,"Trying to load %s\n",argv[1]);
    }else{
      fprintf(stderr,"No specified midi filename! exiting\n");
      return 0;
    }

    am_init();

    pNktSeq=loadSequence(argv[1]);

    if(pNktSeq!=NULL){

        printInfoScreen();
        mainLoop(pNktSeq);

        // destroy sequence
        destroySequence(pNktSeq);
        pNktSeq=0;

        deinstallReplayRout();
        am_deinit(); //deinit our stuff
    }else{
        printf("Error: Loading %s failed.\n",argv[1]);
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
      //install replay rout

    initSequence(pSequence);

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
