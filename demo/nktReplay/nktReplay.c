#include "nkt.h"

#include "input/ikbd.h"
#include "timing/mfp.h"
#include "midi_send.h"

#include <stdio.h>

#ifdef MANUAL_STEP
extern void updateStepNkt(void);
#endif

void printInfoScreen(void);

void mainLoop(sNktSeq *pSequence);

int main(int argc, char *argv[]){
sNktSeq *pNktSeq=0;
int16 iError=0;

    if( (argc>=1) && strlen(argv[1])!='0' ){
        printf("Trying to load %s\n",argv[1]);
    }else{
        printf("No specified nkt filename! exiting\n");
        NktDeinit();
        return 0;
    }

    // hardcoded, todo: set config from commandline
    eMidiDeviceType devType=DT_GS_SOUND_SOURCE;

    switch(devType){
        case DT_LA_SOUND_SOURCE:{
            printf("Configuring MT-32 compatible midi device.\n");
        }break;     /* native MT32 */
        case DT_LA_SOUND_SOURCE_EXT:{
            printf("Configuring extended MT-32 compatible midi device(CM-32L/CM-64).\n");
        }break;   /* for extended MT 32 modules with extra patches like CM-32L/CM-64 */
        case DT_GS_SOUND_SOURCE:{
             printf("Configuring GS compatible midi device.\n");
        }break;       /* for pure GS/GM sound source */
        case DT_LA_GS_MIXED:{
              printf("Configuring GS compatible midi device with LA module.\n");
        }break;           /* if both LA/GS sound sources are available, like in CM-500 */
        case DT_GM_SOUND_SOURCE:{
            printf("Configuring General MIDI compatible device.\n");
        }break;
        case DT_MT32_GM_EMULATION:{
              printf("Configuring MT-32 compatible midi device with GM instrument patch set.\n");
        }break;     /* before loading midi data MT32 sound banks has to be patched */
        case DT_XG_GM_YAMAHA:{
              printf("Configuring XG Yamaha GM device (currently unsupported).\n");
        }break;
        default:{
               printf("Configuring Generic GM/GS compatible midi device.\n");
        }break;
    }

    // set GS / GM source, channel
    NktInit(devType,1);

    pNktSeq=loadSequence(argv[1]);

    if(pNktSeq!=NULL){
        printInfoScreen();
        mainLoop(pNktSeq);

        stopSequence();
        Supexec(NktDeinstallReplayRout);

        // destroy sequence
        destroySequence(pNktSeq);
        pNktSeq=0;

        NktDeinit();

    }else{
        printf("Error: Loading %s failed.\n", argv[1]);
    }

  return 0;
}

void printInfoScreen(void){

  printf("\n===== NKT replay demo v.1.21 =============\n");
  printf("date: %s %s\n",__DATE__,__TIME__);
  printf("    [p] - play loaded tune\n");
  printf("    [r] - pause/unpause played sequence \n");
  printf("    [m] - toggle play once/loop mode\n");
  printf("    [arrow up/down] - adjust master volume\n");
  printf("    [arrow left/right] - adjust master balance\n");
  printf("    [i] - display tune info\n");
  printf("    [h] - show this help screen\n");
  printf("\n    [spacebar] - stop sequence replay \n");
  printf("    [Esc] - quit\n");
  printf("==========================================\n");
  printf("Ready...\n");
}

void displayTuneInfo(void){

  sNktSeq *pPtr=0;
  getCurrentSequence(&pPtr);

  if(pPtr){
    printf("PPQN: %u\t",pPtr->timeDivision);
    printf("Tempo default: %u [ms] last: %u [ms]\n",pPtr->defaultTempo.tempo, pPtr->currentTempo.tempo);
  }

}

void mainLoop(sNktSeq *pSequence){
    bool bQuit=FALSE;
#ifdef MANUAL_STEP
    initSequenceManual(pSequence, NKT_PLAY_ONCE);
#else
    initSequence(pSequence,NKT_PLAY_ONCE,TRUE);
#endif

    //install replay rout
      amMemSet(Ikbd_keyboard, KEY_UNDEFINED, sizeof(Ikbd_keyboard));
      Ikbd_mousex = Ikbd_mousey = Ikbd_mouseb = Ikbd_joystick = 0;

      /* Install our asm ikbd handler */
      Supexec(IkbdInstall);

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

         // adjust master volume
         case SC_ARROW_UP:{

              uint8 _midiMasterVolume=getMidiMasterVolume();

              if(_midiMasterVolume<127){
                  ++_midiMasterVolume;
                  setMidiMasterVolume(_midiMasterVolume);
                  printf("[Master Volume]: %d \n", _midiMasterVolume);
              }

         }break;
          case SC_ARROW_DOWN:{
              uint8 _midiMasterVolume=getMidiMasterVolume();

              if(_midiMasterVolume>0){
                --_midiMasterVolume;
                setMidiMasterVolume(_midiMasterVolume);
                printf("[Master Volume]: %d \n", _midiMasterVolume);
              }

          }break;

          // adjust balance
          case SC_ARROW_LEFT:{

              uint8 _midiMasterBalance=getMidiMasterBalance();

              if(_midiMasterBalance>0){
                  --_midiMasterBalance;
                  setMidiMasterBalance(_midiMasterBalance);
                  printf("<< [Master Pan]: %d \n", _midiMasterBalance);
              }

          }break;
          case SC_ARROW_RIGHT:{

              uint8 _midiMasterBalance=getMidiMasterBalance();

              if(_midiMasterBalance<127){
                  ++_midiMasterBalance;
                  setMidiMasterBalance(_midiMasterBalance);
                  printf("[Master Pan] >>: %d \n", _midiMasterBalance);
              }

          }break;

#ifdef MANUAL_STEP
          case SC_ENTER:{

            for(int i=0;i<200;++i){
                updateStepNkt();
            }

            printNktSequenceState();

            // clear buffer after each update step
            Supexec(flushMidiSendBuffer);

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
