
/**  Copyright 2007-2010 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#include <stdio.h>
#include <ctype.h> 
#include <osbind.h>
#include <stdio.h>
#include <string.h>
#include "amidilib.h"

#include "c_vars.h"
#include "ikbd.h"

// IKBD defines 
#define SC_ESC 0x01
#define SC_CONTROL 0x1d
#define SC_SPACEBAR 0x39

#define SC_Q 0x10
#define SC_A 0x1e
#define SC_W 0x11
#define SC_S 0x1f
#define SC_E 0x12
#define SC_D 0x20
#define SC_R 0x13
#define SC_F 0x21
#define SC_T 0x14
#define SC_G 0x22
#define SC_Y 0x15
#define SC_H 0x23

#define SC_SQ_LEFT_BRACE 0x1a
#define SC_SQ_RIGHT_BRACE 0x1b

#define SC_Z 0x2c
#define SC_X 0x2d
#define SC_LT 0x33
#define SC_GT 0x34

#define SC_1 0x02
#define SC_2 0x03
#define SC_3 0x04
#define SC_4 0x05
#define SC_5 0x06
#define SC_6 0x07
#define SC_7 0x08
#define SC_8 0x09

#define SC_C 0x2e
#define SC_V 0x2f
#define SC_B 0x30
#define SC_I 0x17

static const U8 KEY_PRESSED = 0xff;
static const U8 KEY_UNDEFINED=0x80;
static const U8 KEY_RELEASED=0x00;

extern const char *g_arMIDI2key[];	//midi note to musical tone table 
extern const char *g_arCM32Linstruments[]; //MT32 instruments on 2-9 channel
extern const char *g_arCM32Lrhythm[]; //MT32 rhytm part

extern void turnOffKeyclick(void);

void printHelpScreen(){
  printf("===============================================\n");
  printf("/|\\ midi output test..\n");
  printf("[q-h] - play note\n");
  printf("[1-8] - choose octave \n");
  printf("'[' or ']' - change program number for active channel -/+ \n");
  printf(" [B] - GS source program change for active channel (bank select + program number) -/+ \n");
  printf("'<' or '>' - change active channel/part\n");
  printf("'z' or 'x' - change note velocity -/+ \n");
  printf("[C] - change chorus settings for all channels\n");
  printf("[V] - change reverb settings for all channels\n");
  printf("[i] - show this help screen \n");
  printf("[spacebar] - turn off all sounds \n");
  printf("[Esc] - quit\n");
  printf("(c) Nokturnal 2010\n");
  printf("================================================\n");
}


void changeGSprogramNumber(){
  printf("Change GS program number:\n");
  printf("Bank select for GS sound source\n");
  printf("Select Bank 0-63,44-127 (user area):\n");
  printf("Select Program number:\n");
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
  
  U32 i, quit;
  U8 noteBaseArray[]={24,36,48,60,72,84,96,108};
  U8 currentOctave=3;	
  U8 currentChannel=1;
  U8 currentVelocity=127;
  U8 currentPN=1;
  
  turnOffKeyclick();
  
  /* init library */
  U32 iError=am_init();
 
  //set current channel as 1, default is 0 in external module
  program_change(currentChannel, currentPN);
				  
  printHelpScreen();
  
  memset(Ikbd_keyboard, KEY_UNDEFINED, sizeof(Ikbd_keyboard));
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
				  
				  case SC_B:{
				    changeGSprogramNumber();
				   
				  }break;
				  
				  case SC_I:{
				   printHelpScreen();
				  }break;
 
				}
				
				
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
				  }
				  
				  case SC_E:{
				    note_off(currentChannel,noteBaseArray[currentOctave]+4,currentVelocity);
				  }break;
				  
				  case SC_D:{
				    note_off(currentChannel,noteBaseArray[currentOctave]+5,currentVelocity);
				  }
				  
				  case SC_R:{
				    note_off(currentChannel,noteBaseArray[currentOctave]+6,currentVelocity);
				  }break;
				  
				  case SC_F:{
				    note_off(currentChannel,noteBaseArray[currentOctave]+7,currentVelocity);
				  }
				  
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
				    printf("ch: %d %s (#PC %d)\n",currentChannel,g_arCM32Linstruments[currentPN], currentPN);
				    program_change(currentChannel, currentPN);
				  }break;
				
				  case SC_SPACEBAR:{
				    printf("Silence...\n");
				    am_allNotesOff(16);
				  
				  }break;
				  
				};
				
				
			}
		}
	}

	/* Uninstall our asm handler */
	Supexec(IkbdUninstall);

    /* clean up, free internal library buffers etc..*/
    am_deinit();
   
	
return 0;
}