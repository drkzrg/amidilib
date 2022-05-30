#include "nkt.h"

#include "input/ikbd.h"
#include "timing/mfp.h"
#include "midi_send.h"

#include "core/amprintf.h"

#if AMIDILIB_USE_LIBC
#include <string.h>
#else
#include "amstring.h"
#endif

#ifdef MANUAL_STEP
extern void updateStepNkt(void);
#endif

void printInfoScreen(void);

void mainLoop(sNktSeq *pSequence);

int main(int argc, char *argv[])
{
sNktSeq *pNktSeq=0;
int16 iError=0;

    if( (argc>=1) && strlen(argv[1])!='0' )
    {
        amPrintf("Trying to load %s"NL,argv[1]);
    }
    else
    {
        amPrintf("No specified nkt filename! exiting"NL);
        NktDeinit();
        return 0;
    }

    // hardcoded, todo: set config from commandline
    const eMidiDeviceType devType = DT_GS_SOUND_SOURCE;

    switch(devType){
        case DT_LA_SOUND_SOURCE:{
            amPrintf("Configuring MT-32 compatible midi device."NL);
        }break;     /* native MT32 */
        case DT_LA_SOUND_SOURCE_EXT:{
            amPrintf("Configuring extended MT-32 compatible midi device(CM-32L/CM-64)."NL);
        }break;   /* for extended MT 32 modules with extra patches like CM-32L/CM-64 */
        case DT_GS_SOUND_SOURCE:{
             amPrintf("Configuring GS compatible midi device."NL);
        }break;       /* for pure GS/GM sound source */
        case DT_LA_GS_MIXED:{
              amPrintf("Configuring GS compatible midi device with LA module."NL);
        }break;           /* if both LA/GS sound sources are available, like in CM-500 */
        case DT_GM_SOUND_SOURCE:{
            amPrintf("Configuring General MIDI compatible device."NL);
        }break;
        case DT_MT32_GM_EMULATION:{
              amPrintf("Configuring MT-32 compatible midi device with GM instrument patch set."NL);
        }break;     /* before loading midi data MT32 sound banks has to be patched */
        case DT_XG_GM_YAMAHA:{
              amPrintf("Configuring XG Yamaha GM device (currently unsupported)."NL);
        }break;
        default:{
               amPrintf("Configuring Generic GM/GS compatible midi device."NL);
        }break;
    }

    // set GS / GM source, channel
    NktInit(devType,1);

    pNktSeq=loadNktSequence(argv[1]);

    if(pNktSeq!=NULL)
    {
        printInfoScreen();
        mainLoop(pNktSeq);

        stopNktSequence();
        Supexec(NktDeinstallReplayRout);

        // destroy sequence
        destroyNktSequence(pNktSeq);
        pNktSeq=0;

        NktDeinit();

    }else{
        amPrintf("Error: Loading %s failed."NL, argv[1]);
    }

  return 0;
}

void printInfoScreen(void)
{
  amPrintf(NL "===== NKT replay demo v.1.23 ============="NL);

#if AMIDILIB_USE_LIBC
    amPrintf("build date: %s %s"NL,__DATE__,__TIME__);
#else  
    amPrintf("build date: %s %s nolibc"NL,__DATE__,__TIME__);
#endif

  amPrintf("    [p] - play loaded tune"NL);
  amPrintf("    [r] - pause/unpause played sequence "NL);
  amPrintf("    [m] - toggle play once/loop mode"NL);
  amPrintf("    [arrow up/down] - adjust master volume"NL);
  amPrintf("    [arrow left/right] - adjust master balance"NL);
  amPrintf("    [i] - display tune info"NL);
  amPrintf("    [h] - show this help screen"NL);
  amPrintf(NL "    [spacebar] - stop sequence replay "NL);
  amPrintf("    [Esc] - quit"NL);
  amPrintf("(c) Nokturnal 2007-22"NL);   
  amPrintf("=========================================="NL);
  amPrintf("Ready..."NL);
}

void displayTuneInfo(void)
{
  const sNktSeq *pPtr = getActiveNktSequence();

  amPrintf("PPQN: %u\t",pPtr->timeDivision);
  amPrintf("Tempo default: %u [ms] last: %u [ms]"NL,pPtr->defaultTempo.tempo, pPtr->currentTempo.tempo);
}

void mainLoop(sNktSeq *pSequence)
{
    bool bQuit=FALSE;
#ifdef MANUAL_STEP
    initNktSequenceManual(pSequence, NKT_PLAY_ONCE);
#else
    initNktSequence(pSequence,NKT_PLAY_ONCE,TRUE);
#endif

      IkbdClearState();

      // Install our asm ikbd handler 
      Supexec(IkbdInstall);

      //####
      while(bQuit!=TRUE)
      {
        // check keyboard input
        for (uint16 i=0; i<IKBD_TABLE_SIZE; ++i) 
        {

          if (Ikbd_keyboard[i]==KEY_PRESSED) 
          {
          Ikbd_keyboard[i]=KEY_UNDEFINED;

          switch(i){
        case SC_ESC:{
          bQuit=TRUE;
          //stop sequence
          stopNktSequence();
        }break;
        case SC_P:{
          //starts playing sequence if is stopped
          playNktSequence();
         }break;
        case SC_R:{
          //pauses sequence when is playing
           pauseNktSequence();
         }break;
         case SC_M:{
          //toggles between play once and play in loop
          switchNktReplayMode();
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
                  amPrintf("[Master Volume]: %d "NL, _midiMasterVolume);
              }

         }break;
          case SC_ARROW_DOWN:{
              uint8 _midiMasterVolume=getMidiMasterVolume();

              if(_midiMasterVolume>0){
                --_midiMasterVolume;
                setMidiMasterVolume(_midiMasterVolume);
                amPrintf("[Master Volume]: %d "NL, _midiMasterVolume);
              }

          }break;

          // adjust balance
          case SC_ARROW_LEFT:{

              uint8 _midiMasterBalance=getMidiMasterBalance();

              if(_midiMasterBalance>0){
                  --_midiMasterBalance;
                  setMidiMasterBalance(_midiMasterBalance);
                  amPrintf("<< [Master Pan]: %d "NL, _midiMasterBalance);
              }

          }break;
          case SC_ARROW_RIGHT:{

              uint8 _midiMasterBalance=getMidiMasterBalance();

              if(_midiMasterBalance<127){
                  ++_midiMasterBalance;
                  setMidiMasterBalance(_midiMasterBalance);
                  amPrintf("[Master Pan] >>: %d "NL, _midiMasterBalance);
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
            stopNktSequence();
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
