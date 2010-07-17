
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
#define SCANCODE_ESC	0x01
#define KEY_PRESSED	0xff
#define KEY_UNDEFINED	0x80
#define KEY_RELEASED	0x00

extern void turnOffKeyclick(void);

void printHelpScreen(){
  printf("===============================================\n");
  printf("/|\\ midi output test..\n");
  printf("'[' or ']' - change octave -/+ \n");
  printf("[q-h] - play note\n");
  printf("'z' or 'x' - change channels amplitude -/+ \n");
  printf("[i] - show this help screen \n");
  printf("[spacebar] - turn off all sounds \n");
  printf("[Esc] - quit\n");
  printf("(c)Nokturnal 2010\n");
  printf("================================================\n");
}

int main(void) {
  	int i, quit;

  turnOffKeyclick();
  
  printHelpScreen();
  
  U8 noteBaseArray[]={24,36,48,60,72,84,96,108};
  
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
				if (i==SCANCODE_ESC) {
					quit=1;
				}
			}
			if (Ikbd_keyboard[i]==KEY_RELEASED) {
				printf("Key with scancode 0x%02x released\n", i);
				Ikbd_keyboard[i]=KEY_UNDEFINED;
			}
		}
	}

	/* Uninstall our asm handler */
	Supexec(IkbdUninstall);

return 0;
}