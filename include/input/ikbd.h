#ifndef IKBD_H
#define IKBD_H

//	IKBD 6301 interrupt routine
//	Copyright (C) 2002	Patrice Mandin
//  Copyright (C) 2007-22 Paweł Góralski

#include "scancode.h"
#include "vartypes.h"

// Const 
#define IKBD_JOY_UP	(1<<0)
#define IKBD_JOY_DOWN	(1<<1)
#define IKBD_JOY_LEFT	(1<<2)
#define IKBD_JOY_RIGHT	(1<<3)
#define IKBD_JOY_FIRE	(1<<7)

#define IKBD_TABLE_SIZE 128

// Variables 
AM_EXTERN uint8		Ikbd_keyboard[IKBD_TABLE_SIZE];		// Keyboard table 
AM_EXTERN uint16	Ikbd_mouseb;						// Mouse on port 0, buttons 
AM_EXTERN int16 	Ikbd_mousex;						// Mouse X relative motion 
AM_EXTERN int16 	Ikbd_mousey;						// Mouse Y relative motion 
AM_EXTERN uint16	Ikbd_joystick;						// Joystick on port 1 
				
// Functions  
AM_EXTERN void IkbdClearState(void);
AM_EXTERN void IkbdInstall(void);
AM_EXTERN void IkbdUninstall(void);
AM_EXTERN void turnOffKeyclick(void);

static const uint8 KEY_PRESSED = 0xff;
static const uint8 KEY_UNDEFINED = 0x80;
static const uint8 KEY_RELEASED = 0x00;

#endif /* IKBD_H */
