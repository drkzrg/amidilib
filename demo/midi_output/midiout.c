
/**  Copyright 2007-2022 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#if AMIDILIB_USE_LIBC
#include <string.h>
#else
#include "amstring.h"
#endif

#include "vartypes.h"
#include "amidilib.h"
#include "config.h"
#include "roland.h"
#include "rolinstr.h"
#include "midi.h"

#include "core/amprintf.h"
#include "input/scancode.h"	// scancode definitions
#include "input/ikbd.h"

void printHelpScreen(void)
{
  amPrintf("==============================================="NL);
  amPrintf("/|\\ midi output test.."NL);
  amPrintf("[q-h] - play note"NL);
  amPrintf("[1-8] - choose octave "NL);
  amPrintf("'[' or ']' - change program number for active channel -/+ "NL);
  amPrintf(" [B] - change instrument for active channel" NL "\t -/+ "NL);
  amPrintf(" [Arrow Up/Down] - adjust bank select -/+ "NL);
  amPrintf("'<' or '>' - change active channel/part"NL);
  amPrintf("'z' or 'x' - change note velocity -/+ "NL);
  amPrintf("[C] - change chorus settings for all channels"NL);
  amPrintf("[V] - change reverb settings for all channels"NL);
  amPrintf("[HELP] - show this help screen "NL);
  amPrintf("[Backspace] - switch between [LA] / [GS/GM] mode "NL);
  
  switch(getGlobalConfig()->connectedDeviceType){
    case DT_LA_SOUND_SOURCE:     
    case DT_LA_SOUND_SOURCE_EXT:{
      amPrintf("[current mode] - LA synth  "NL);
    }break;
    
    case DT_GS_SOUND_SOURCE:       /* for pure GS/GM sound source */
    case DT_LA_GS_MIXED:           /* if both LA/GS sound sources are available, like in CM-500 */
    case DT_MT32_GM_EMULATION:     /* before loading midi data MT32 sound banks has to be patched */
    case DT_XG_GM_YAMAHA:{
    amPrintf("[current mode] - GS/GM synth"NL);
    }break;
    
   }  
  
  amPrintf("[spacebar] - turn off all sounds "NL);
  amPrintf("[Esc] - quit"NL);
  amPrintf("(c) Nokturnal 2007-2022"NL);
  amPrintf("================================================"NL);
}


void changeCurrentInstrument(const uint8 channel,const uint8 bank, const uint8 pn)
{  
    
  switch(getGlobalConfig()->connectedDeviceType){
    case DT_LA_SOUND_SOURCE:     
    case DT_LA_SOUND_SOURCE_EXT:{
      amPrintf("\nSetting LA instrument pn: [%d] on ch: [%d]"NL, pn, channel);
      program_change(channel, pn);
    }break;
    
    case DT_GS_SOUND_SOURCE:       /* for pure GS/GM sound source */
    case DT_LA_GS_MIXED:           /* if both LA/GS sound sources are available, like in CM-500 */
    case DT_MT32_GM_EMULATION:     /* before loading midi data MT32 sound banks has to be patched */
    case DT_XG_GM_YAMAHA:
    default:
    {
      amPrintf("\nSetting GS/GM instrument bank:[%d]: pn [%d]: on [ch]: %d"NL, bank, pn, channel);

      control_change(C_BANK_SELECT, channel, bank,0x00);
      program_change(channel, pn);
    }break;
    
   }  
  
  #ifdef IKBD_MIDI_SEND_DIRECT
    Supexec(flushMidiSendBuffer);
  #endif
}


void changeGlobalChorusSettings(void)
{
  amPrintf("Change global chorus settings:"NL);
}

void changeGlobalReverbSettings(void){
  amPrintf("Change global reverb settings:"NL);
}

void increaseGlobalMasterVolume(void){
  amPrintf("Increase global Master volume"NL);
}

void decreaseGlobalMasterVolume(void){
  amPrintf("Decrease global Master volume"NL);
}

//======================================================================================================
int main(void) {
  uint32 i, quit;
  uint8 noteBaseArray[]={24,36,48,60,72,84,96,108};
  uint8 currentOctave=3;	
  uint8 currentVelocity=127;
  uint8 currentChannel=0;
  uint8 currentPN=1;
  uint8 currentBankSelect=0;

  turnOffKeyclick();

  /* init library */
  retVal iError=amInit();
 
  if(iError!=AM_OK) return -1;
  
#ifdef IKBD_MIDI_SEND_DIRECT
    Supexec(flushMidiSendBuffer);
#endif
  
  currentChannel=getGlobalConfig()->midiChannel;  
    
  printHelpScreen();

  amMemSet(Ikbd_keyboard, KEY_UNDEFINED, sizeof(Ikbd_keyboard));
  Ikbd_mousex = Ikbd_mousey = Ikbd_mouseb = Ikbd_joystick = 0;

	/* Install our asm handler */
	Supexec(IkbdInstall);
	amPrintf("Press ESC to quit"NL);

	/* Wait till ESC key pressed */
	quit = 0;
	while (!quit) {
		for (i=0; i<128; i++) {
			if (Ikbd_keyboard[i]==KEY_PRESSED) {
				
			     Ikbd_keyboard[i]=KEY_UNDEFINED;
				
				switch(i){
				  case SC_ESC:{
				    quit=1;
				  }break;
				  				  //change octave
				  case SC_1:{
				    amPrintf("octave: -3 set"NL);
				   
				    currentOctave=0;
				  }break;
				  //change octave
				  case SC_2:{
				  amPrintf("octave: -2 set"NL);
				   
				    currentOctave=1;
				  }break;
				  
				  //change octave
				  case SC_3:{
				  amPrintf("octave: -1 set "NL);
				   
				    currentOctave=2;
				  }break;
				  //change octave
				  case SC_4:{
				    amPrintf("octave: 0 set"NL);
				   
				  currentOctave=3;
				  }break;
				  //change octave
				  case SC_5:{
				    amPrintf("octave: 1 set "NL);
				   
				  currentOctave=4;
				  }break;
				  
				  //change octave
				  case SC_6:{
				    amPrintf("octave: 2 set "NL);
				   
				  currentOctave=5;
				  }break;
				  
				  //change octave
				  case SC_7:{
				    amPrintf("octave: 3 set"NL);
				   
				  currentOctave=6;
				  }break;
				  //change octave
				  case SC_8:{
				    amPrintf("octave: 4 set"NL);
				   
				  currentOctave=7;
				  }break;
				  //note on handling
				  case SC_Q:{
				    note_on(currentChannel,noteBaseArray[currentOctave]+0,currentVelocity);
				    amPrintf("%s"NL,getNoteName(currentChannel,currentPN,noteBaseArray[currentOctave]+0));
				  }break;
				  case SC_A:{
				    note_on(currentChannel,noteBaseArray[currentOctave]+1,currentVelocity);
    				    amPrintf("%s"NL,getNoteName(currentChannel,currentPN,noteBaseArray[currentOctave]+1));
				  }break;
				  case SC_W:{
				    note_on(currentChannel,noteBaseArray[currentOctave]+2,currentVelocity);
       				    amPrintf("%s"NL,getNoteName(currentChannel,currentPN,noteBaseArray[currentOctave]+2));
				  }break;
				  case SC_S:{
				    note_on(currentChannel,noteBaseArray[currentOctave]+3,currentVelocity);
       				    amPrintf("%s"NL,getNoteName(currentChannel,currentPN,noteBaseArray[currentOctave]+3));
				  }break;
				  case SC_E:{
				    note_on(currentChannel,noteBaseArray[currentOctave]+4,currentVelocity);
       				    amPrintf("%s"NL,getNoteName(currentChannel,currentPN,noteBaseArray[currentOctave]+4));
				  }break;
				  
				  case SC_D:{
				    note_on(currentChannel,noteBaseArray[currentOctave]+5,currentVelocity);
       				    amPrintf("%s"NL,getNoteName(currentChannel,currentPN,noteBaseArray[currentOctave]+5));
				  }break;
				  
				  case SC_R:{
				    note_on(currentChannel,noteBaseArray[currentOctave]+6,currentVelocity);
       				    amPrintf("%s"NL,getNoteName(currentChannel,currentPN,noteBaseArray[currentOctave]+6));
				  }break;
				  
				  case SC_F:{
				    note_on(currentChannel,noteBaseArray[currentOctave]+7,currentVelocity);
       				    amPrintf("%s"NL,getNoteName(currentChannel,currentPN,noteBaseArray[currentOctave]+7));
				  }break;
				  
				  case SC_T:{
				    note_on(currentChannel,noteBaseArray[currentOctave]+8,currentVelocity);
       				    amPrintf("%s"NL,getNoteName(currentChannel,currentPN,noteBaseArray[currentOctave]+8));
				  }break;
				  
				  case SC_G:{
				    note_on(currentChannel,noteBaseArray[currentOctave]+9,currentVelocity);				        				    
				    amPrintf("%s"NL,getNoteName(currentChannel,currentPN,noteBaseArray[currentOctave]+9));
				  }break;
				  
				  case SC_Y:{
				    note_on(currentChannel,noteBaseArray[currentOctave]+10,currentVelocity);
       				    amPrintf("%s"NL,getNoteName(currentChannel,currentPN,noteBaseArray[currentOctave]+10));
				  }break;
				  
				  case SC_H:{
				     note_on(currentChannel,noteBaseArray[currentOctave]+11,currentVelocity);
       				     amPrintf("%s"NL,getNoteName(currentChannel,currentPN,noteBaseArray[currentOctave]+11));
				  }break;
				  
				  // change program number
				  case SC_SQ_LEFT_BRACE:
				  {
				    if(currentPN!=1){
				      currentPN--;
				    }else{
				      currentPN=128;
				    }
				     
				  }break;
				  
				  case SC_SQ_RIGHT_BRACE:{
				  if(currentPN!=128){
				      currentPN++;
				    }else{
				      currentPN=1;
				    }
				  }break;
				  //change velocity
				  case SC_Z :{
				    if(currentVelocity!=0){
				      currentVelocity--;
				    }else{
				      currentVelocity=127;
				    }
				    amPrintf("Current note velocity:: %d "NL,currentVelocity);
				  }break;
		
				  case SC_X:{
				    if(currentVelocity!=127){
				      currentVelocity++;
				    }else{
				      currentVelocity=0;
				    }
				    amPrintf("Current note velocity:: %d "NL,currentVelocity);
				  }break;
				  
				  //change active channel/part 0-15
				  case SC_LT: {
				    if(currentChannel!=0){
				      amAllNotesOff(16);
				      currentChannel--;
				      program_change(currentChannel, currentPN);
                      amPrintf("active channel: %d "NL,currentChannel);
				    }
				    
				  }break;
				 
				  case SC_GT:{
				    if(currentChannel!=15){
                        amAllNotesOff(16);
                        currentChannel++;
				        program_change(currentChannel, currentPN);
                        amPrintf("Current channel: %d "NL,currentChannel);
				    }
				  }break;
				  case SC_ARROW_UP:{
				    if(currentBankSelect!=127){
				      currentBankSelect++;
				    }
				    else{
				      currentBankSelect=0;
				    }
				  amPrintf("Current bank: %d "NL,currentBankSelect);
				  }break;
				  case SC_ARROW_DOWN:{
				    if(currentBankSelect!=0){
				      currentBankSelect--;
				    }
				    else{
				      currentBankSelect=127;
				    }
				  amPrintf("Current bank: %d "NL,currentBankSelect);
				  
				  }break;
				  case SC_B:{
				    changeCurrentInstrument(currentChannel,currentBankSelect,currentPN);
				  }break;
				  
				  case SC_HELP:{
				   printHelpScreen();
				  }break;
				  
				  case SC_BACKSPACE:{
				    switch(getGlobalConfig()->connectedDeviceType){
				      case DT_LA_SOUND_SOURCE:     
				      case DT_LA_SOUND_SOURCE_EXT:{
                            setConnectedDeviceType(DT_GS_SOUND_SOURCE);
                            amPrintf("Set MT32 mode."NL);
				      }break;
    
				      case DT_GS_SOUND_SOURCE:       
				      case DT_LA_GS_MIXED:           
				      case DT_MT32_GM_EMULATION:     
				      case DT_XG_GM_YAMAHA:{
					setConnectedDeviceType(DT_LA_SOUND_SOURCE_EXT);
					amPrintf("Set GS/GM mode."NL);
				    }break;
    
				  }  
				    
				  }break;
				  
				}
			#ifdef IKBD_MIDI_SEND_DIRECT
                 Supexec(flushMidiSendBuffer);
			#endif
				  	
				
			}
			if (Ikbd_keyboard[i]==KEY_RELEASED) {
				
			  Ikbd_keyboard[i]=KEY_UNDEFINED;
				
				switch(i)
				{
				  //note off handling
				  case SC_Q:{
				  note_off(currentChannel,noteBaseArray[currentOctave]+0,currentVelocity);
				    
				  }break;
				  
				  case SC_A:{
				    note_off(currentChannel,noteBaseArray[currentOctave]+1,currentVelocity);
				  
				  }break;
				  
				  case SC_W:{
				    note_off(currentChannel,noteBaseArray[currentOctave]+2,currentVelocity);
				  }break;
				  
				  case SC_S:{
				    note_off(currentChannel,noteBaseArray[currentOctave]+3,currentVelocity);
          }break;
				  
				  case SC_E:{
				    note_off(currentChannel,noteBaseArray[currentOctave]+4,currentVelocity);
				  }break;
				  
				  case SC_D:{
				    note_off(currentChannel,noteBaseArray[currentOctave]+5,currentVelocity);
          }break;
				  
				  case SC_R:{
				    note_off(currentChannel,noteBaseArray[currentOctave]+6,currentVelocity);
				  }break;
				  
				  case SC_F:{
				    note_off(currentChannel,noteBaseArray[currentOctave]+7,currentVelocity);
                  }break;
				  
				  case SC_T:{
				    note_off(currentChannel,noteBaseArray[currentOctave]+8,currentVelocity);
				  }break;
				  
				  case SC_G:{
				    note_off(currentChannel,noteBaseArray[currentOctave]+9,currentVelocity);
				  }break;
				  
				  case SC_Y:{
				    note_off(currentChannel,noteBaseArray[currentOctave]+10,currentVelocity);
				  }break;
				  
				  case SC_H:{
				    note_off(currentChannel,noteBaseArray[currentOctave]+11,currentVelocity);
				  }break;
		
				  // send chosen program number
				  case SC_SQ_LEFT_BRACE:
				  case SC_SQ_RIGHT_BRACE:{

                      switch(getGlobalConfig()->connectedDeviceType){
				      case DT_LA_SOUND_SOURCE:     
				      case DT_LA_SOUND_SOURCE_EXT:{
                        amPrintf("ch: [%d] [%s] (#PC %d)"NL,currentChannel,getCM32LInstrName(currentPN), currentPN);
                        program_change(currentChannel, currentPN);
				      }break;
    
				      case DT_GS_SOUND_SOURCE:       /* for pure GS/GM sound source */
				      case DT_LA_GS_MIXED:           /* if both LA/GS sound sources are available, like in CM-500 */
				      case DT_MT32_GM_EMULATION:     /* before loading midi data MT32 sound banks has to be patched */
				      case DT_XG_GM_YAMAHA:
                      case DT_GM_SOUND_SOURCE:
				      default:{
                        amPrintf("ch: [%d] b: [%d] [%s] (#PC %d)"NL,currentChannel,currentBankSelect, getCM32LInstrName(currentPN), currentPN);
				    
                        control_change(C_BANK_SELECT, currentChannel,currentBankSelect,0x00);
                        program_change(currentChannel, currentPN);
				      }break;
				    }
				    
				  }break;
				
				  case SC_SPACEBAR:{
				    amPrintf("Silence..."NL);
				    amAllNotesOff(16);
				  
				  }break;
				  
				};
			  
			  #ifdef IKBD_MIDI_SEND_DIRECT
          Supexec(flushMidiSendBuffer);
			  #endif
			}
		}
	}

      /* Uninstall our asm handler */
	Supexec(IkbdUninstall);

/* clean up, free internal library buffers etc..*/
 amDeinit();
 return 0;
}
