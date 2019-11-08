
/**  Copyright 2007-2019 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#include <stdio.h>
#include <string.h>
#include <stdio.h>

#include "c_vars.h"
#include "amidilib.h"
#include "config.h"
#include "roland.h"
#include "rolinstr.h"
#include "midi.h"

#include "input/scancode.h"	// scancode definitions
#include "input/ikbd.h"

void printHelpScreen(void){
  printf("===============================================\n");
  printf("/|\\ midi output test..\n");
  printf("[q-h] - play note\n");
  printf("[1-8] - choose octave \n");
  printf("'[' or ']' - change program number for active channel -/+ \n");
  printf(" [B] - change instrument for active channel\n\t -/+ \n");
  printf(" [Arrow Up/Down] - adjust bank select -/+ \n");
  printf("'<' or '>' - change active channel/part\n");
  printf("'z' or 'x' - change note velocity -/+ \n");
  printf("[C] - change chorus settings for all channels\n");
  printf("[V] - change reverb settings for all channels\n");
  printf("[HELP] - show this help screen \n");
  printf("[Backspace] - switch between [LA] / [GS/GM] mode \n");
  
  switch(getGlobalConfig()->connectedDeviceType){
    case DT_LA_SOUND_SOURCE:     
    case DT_LA_SOUND_SOURCE_EXT:{
      printf("[current mode] - LA synth  \n");
    }break;
    
    case DT_GS_SOUND_SOURCE:       /* for pure GS/GM sound source */
    case DT_LA_GS_MIXED:           /* if both LA/GS sound sources are available, like in CM-500 */
    case DT_MT32_GM_EMULATION:     /* before loading midi data MT32 sound banks has to be patched */
    case DT_XG_GM_YAMAHA:{
    printf("[current mode] - GS/GM synth\n");
    }break;
    
   }  
  
  printf("[spacebar] - turn off all sounds \n");
  printf("[Esc] - quit\n");
  printf("(c) Nokturnal 2013\n");
  printf("================================================\n");
}


void changeCurrentInstrument(const uint8 channel,const uint8 bank, const uint8 pn){  
    
  switch(getGlobalConfig()->connectedDeviceType){
    case DT_LA_SOUND_SOURCE:     
    case DT_LA_SOUND_SOURCE_EXT:{
      printf("\nSetting LA instrument pn: [%d] on ch: [%d]\n", pn, channel);
      program_change(channel, pn);
    }break;
    
    case DT_GS_SOUND_SOURCE:       /* for pure GS/GM sound source */
    case DT_LA_GS_MIXED:           /* if both LA/GS sound sources are available, like in CM-500 */
    case DT_MT32_GM_EMULATION:     /* before loading midi data MT32 sound banks has to be patched */
    case DT_XG_GM_YAMAHA:
    default:{
      printf("\nSetting GS/GM instrument bank:[%d]: pn [%d]: on [ch]: %d\n", bank, pn, channel);

      control_change(C_BANK_SELECT, channel, bank,0x00);
      program_change(channel, pn);
    }break;
    
   }  
  
  #ifdef IKBD_MIDI_SEND_DIRECT
    Supexec(flushMidiSendBuffer);
  #endif
}


void changeGlobalChorusSettings(){
  printf("Change global chorus settings:\n");
}

void changeGlobalReverbSettings(){
  printf("Change global reverb settings:\n");
}

void increaseGlobalMasterVolume(){
  printf("Increase global Master volume\n");
}

void decreaseGlobalMasterVolume(){
  printf("Decrease global Master volume\n");
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
  uint32 iError=am_init();
 
  if(iError!=1) return -1;
  
#ifdef IKBD_MIDI_SEND_DIRECT
    Supexec(flushMidiSendBuffer);
#endif
  
  currentChannel=getGlobalConfig()->midiChannel;  
    
  printHelpScreen();

  amMemSet(Ikbd_keyboard, KEY_UNDEFINED, sizeof(Ikbd_keyboard));
  Ikbd_mousex = Ikbd_mousey = Ikbd_mouseb = Ikbd_joystick = 0;

	/* Install our asm handler */
	Supexec(IkbdInstall);
	printf("Press ESC to quit\n");

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
				    printf("octave: -3 set\n");
				   
				    currentOctave=0;
				  }break;
				  //change octave
				  case SC_2:{
				  printf("octave: -2 set\n");
				   
				    currentOctave=1;
				  }break;
				  
				  //change octave
				  case SC_3:{
				  printf("octave: -1 set \n");
				   
				    currentOctave=2;
				  }break;
				  //change octave
				  case SC_4:{
				    printf("octave: 0 set\n");
				   
				  currentOctave=3;
				  }break;
				  //change octave
				  case SC_5:{
				    printf("octave: 1 set \n");
				   
				  currentOctave=4;
				  }break;
				  
				  //change octave
				  case SC_6:{
				    printf("octave: 2 set \n");
				   
				  currentOctave=5;
				  }break;
				  
				  //change octave
				  case SC_7:{
				    printf("octave: 3 set\n");
				   
				  currentOctave=6;
				  }break;
				  //change octave
				  case SC_8:{
				    printf("octave: 4 set\n");
				   
				  currentOctave=7;
				  }break;
				  //note on handling
				  case SC_Q:{
				    note_on(currentChannel,noteBaseArray[currentOctave]+0,currentVelocity);
				    printf("%s\n",getNoteName(currentChannel,currentPN,noteBaseArray[currentOctave]+0));
				  }break;
				  case SC_A:{
				    note_on(currentChannel,noteBaseArray[currentOctave]+1,currentVelocity);
    				    printf("%s\n",getNoteName(currentChannel,currentPN,noteBaseArray[currentOctave]+1));
				  }break;
				  case SC_W:{
				    note_on(currentChannel,noteBaseArray[currentOctave]+2,currentVelocity);
       				    printf("%s\n",getNoteName(currentChannel,currentPN,noteBaseArray[currentOctave]+2));
				  }break;
				  case SC_S:{
				    note_on(currentChannel,noteBaseArray[currentOctave]+3,currentVelocity);
       				    printf("%s\n",getNoteName(currentChannel,currentPN,noteBaseArray[currentOctave]+3));
				  }break;
				  case SC_E:{
				    note_on(currentChannel,noteBaseArray[currentOctave]+4,currentVelocity);
       				    printf("%s\n",getNoteName(currentChannel,currentPN,noteBaseArray[currentOctave]+4));
				  }break;
				  
				  case SC_D:{
				    note_on(currentChannel,noteBaseArray[currentOctave]+5,currentVelocity);
       				    printf("%s\n",getNoteName(currentChannel,currentPN,noteBaseArray[currentOctave]+5));
				  }break;
				  
				  case SC_R:{
				    note_on(currentChannel,noteBaseArray[currentOctave]+6,currentVelocity);
       				    printf("%s\n",getNoteName(currentChannel,currentPN,noteBaseArray[currentOctave]+6));
				  }break;
				  
				  case SC_F:{
				    note_on(currentChannel,noteBaseArray[currentOctave]+7,currentVelocity);
       				    printf("%s\n",getNoteName(currentChannel,currentPN,noteBaseArray[currentOctave]+7));
				  }break;
				  
				  case SC_T:{
				    note_on(currentChannel,noteBaseArray[currentOctave]+8,currentVelocity);
       				    printf("%s\n",getNoteName(currentChannel,currentPN,noteBaseArray[currentOctave]+8));
				  }break;
				  
				  case SC_G:{
				    note_on(currentChannel,noteBaseArray[currentOctave]+9,currentVelocity);				        				    
				    printf("%s\n",getNoteName(currentChannel,currentPN,noteBaseArray[currentOctave]+9));
				  }break;
				  
				  case SC_Y:{
				    note_on(currentChannel,noteBaseArray[currentOctave]+10,currentVelocity);
       				    printf("%s\n",getNoteName(currentChannel,currentPN,noteBaseArray[currentOctave]+10));
				  }break;
				  
				  case SC_H:{
				     note_on(currentChannel,noteBaseArray[currentOctave]+11,currentVelocity);
       				     printf("%s\n",getNoteName(currentChannel,currentPN,noteBaseArray[currentOctave]+11));
				  }break;
				  
				  // change program number
				  case SC_SQ_LEFT_BRACE:{
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
				    printf("Current note velocity:: %d \n",currentVelocity);
				  }break;
		
				  case SC_X:{
				    if(currentVelocity!=127){
				      currentVelocity++;
				    }else{
				      currentVelocity=0;
				    }
				    printf("Current note velocity:: %d \n",currentVelocity);
				  }break;
				  
				  //change active channel/part 0-15
				  case SC_LT: {
				    if(currentChannel!=0){
				      am_allNotesOff(16);
				      currentChannel--;
				      program_change(currentChannel, currentPN);
                      printf("active channel: %d \n",currentChannel);
				    }
				    
				  }break;
				 
				  case SC_GT:{
				    if(currentChannel!=15){
                        am_allNotesOff(16);
                        currentChannel++;
				        program_change(currentChannel, currentPN);
                        printf("Current channel: %d \n",currentChannel);
				    }
				  }break;
				  case SC_ARROW_UP:{
				    if(currentBankSelect!=127){
				      currentBankSelect++;
				    }
				    else{
				      currentBankSelect=0;
				    }
				  printf("Current bank: %d \n",currentBankSelect);
				  }break;
				  case SC_ARROW_DOWN:{
				    if(currentBankSelect!=0){
				      currentBankSelect--;
				    }
				    else{
				      currentBankSelect=127;
				    }
				  printf("Current bank: %d \n",currentBankSelect);
				  
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
                            printf("Set MT32 mode.\n");
				      }break;
    
				      case DT_GS_SOUND_SOURCE:       
				      case DT_LA_GS_MIXED:           
				      case DT_MT32_GM_EMULATION:     
				      case DT_XG_GM_YAMAHA:{
					setConnectedDeviceType(DT_LA_SOUND_SOURCE_EXT);
					printf("Set GS/GM mode.\n");
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
				
				switch(i){
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
                        printf("ch: [%d] [%s] (#PC %d)\n",currentChannel,getCM32LInstrName(currentPN), currentPN);
                        program_change(currentChannel, currentPN);
				      }break;
    
				      case DT_GS_SOUND_SOURCE:       /* for pure GS/GM sound source */
				      case DT_LA_GS_MIXED:           /* if both LA/GS sound sources are available, like in CM-500 */
				      case DT_MT32_GM_EMULATION:     /* before loading midi data MT32 sound banks has to be patched */
				      case DT_XG_GM_YAMAHA:
                      case DT_GM_SOUND_SOURCE:
				      default:{
                        printf("ch: [%d] b: [%d] [%s] (#PC %d)\n",currentChannel,currentBankSelect, getCM32LInstrName(currentPN), currentPN);
				    
                        control_change(C_BANK_SELECT, currentChannel,currentBankSelect,0x00);
                        program_change(currentChannel, currentPN);
				      }break;
				    }
				    
				  }break;
				
				  case SC_SPACEBAR:{
				    printf("Silence...\n");
				    am_allNotesOff(16);
				  
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
 am_deinit();
 return 0;
}


