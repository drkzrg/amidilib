
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


static const U8 KEY_PRESSED = 0xff;
static const U8 KEY_UNDEFINED=0x80;
static const U8 KEY_RELEASED=0x00;

extern void turnOffKeyclick(void);

void printHelpScreen(){
  printf("===============================================\n");
  printf("/|\\ midi output test..\n");
  printf("[q-h] - play note\n");
  printf("[1-8] - choose octave \n");
  printf("'[' or ']' - change program number for active channel -/+ \n");
  printf("'<' or '>' - change active channel\n");
  printf("'z' or 'x' - change note velocity -/+ \n");
  printf("[i] - show this help screen \n");
  printf("[spacebar] - turn off all sounds \n");
  printf("[Esc] - quit\n");
  printf("(c) Nokturnal 2010\n");
  printf("================================================\n");
}

int main(void) {
  U32 i, quit;
  U8 noteBaseArray[]={24,36,48,60,72,84,96,108};

  turnOffKeyclick();
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
				
				printf("Key with scancode 0x%02x pressed\n", i);
				Ikbd_keyboard[i]=KEY_UNDEFINED;
				
				switch(i){
				  case SC_ESC:{
				    quit=1;
				  }break;
				  case SC_SPACEBAR:{
				    printf("Turn all sounds off\n");
				  }break;
				  
				  //change octave
				  case SC_1:{
				  
				  }break;
				  //change octave
				  case SC_2:{
				  
				  }break;
				  
				  //change octave
				  case SC_3:{
				  
				  }break;
				  
				  //change octave
				  case SC_4:{
				  
				  }break;
				  //change octave
				  case SC_5:{
				  
				  }break;
				  
				  //change octave
				  case SC_6:{
				  
				  }break;
				  
				  //change octave
				  case SC_7:{
				  
				  }break;
				  
				  //change octave
				  case SC_8:{
				  
				  }break;
				  
				  
				  //note on handling
				  case SC_Q:{
				  
				  }
				  case SC_A:{
				  
				  }
				  case SC_W:{
				  
				  }
				  case SC_S:{
				  
				  }
				  case SC_E:{
				  
				  }
				  
				  case SC_D:{
				  
				  }
				  
				  case SC_R:{
				  
				  }
				  
				  case SC_F:{
				  
				  
				  }
				  
				  case SC_T:{
				  
				  }
				  
				  case SC_G:{
				  
				  }
				  
				  case SC_Y:{
				  
				  }
				  
				  case SC_H:{
				  
				  }
				  
				  // change program number
				  case SC_SQ_LEFT_BRACE:{
				  
				  }
				  
				  case SC_SQ_RIGHT_BRACE:{
				  
				  }
				  //change velocity
				  case SC_Z :{
				  
				  }
		
				  case SC_X:{
				  
				  }
				  
				  //change active channel 0-15
				  case SC_LT: {
				  
				  }
				 
				  case SC_GT:{
				  
				  }
 
				}
				
				
			}
			if (Ikbd_keyboard[i]==KEY_RELEASED) {
				printf("Key with scancode 0x%02x released\n", i);
				Ikbd_keyboard[i]=KEY_UNDEFINED;
				
				switch(i){
				  //note off handling
				  case SC_Q:{
				  
				  }
				  
				  case SC_A:{
				  
				  }
				  
				  case SC_W:{
				  
				  }
				  
				  case SC_S:{
				  
				  }
				  
				  case SC_E:{
				  
				  }
				  
				  case SC_D:{
				  
				  }
				  
				  case SC_R:{
				  
				  }
				  
				  case SC_F:{
				  
				  }
				  
				  case SC_T:{
				  
				  }
				  
				  case SC_G:{
				  
				  }
				  
				  case SC_Y:{
				  
				  }
				  
				  case SC_H:{
				  
				  }
		
				  // send chosen program number
				  case SC_SQ_LEFT_BRACE:{
				  
				  }break;
				  
				  case SC_SQ_RIGHT_BRACE:{
				  
				  }break;
				
				  
				  
				};
				
				
			}
		}
	}

	/* Uninstall our asm handler */
	Supexec(IkbdUninstall);

return 0;
}