/**  Copyright 2007-2010 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

// configuration managment, save/load settings from file
// here we can specify used device type, event memory pool size

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "c_vars.h"

typedef struct _amidiConfig{
  U16 version;			//config version
  U32 configSize;		//sizeof tAmidiConfig
  U16 connectedDeviceType;	//as in eMidiDeviceTypes
  U16 operationMode;		//we can have connected device  
  U16 midiChannel;		//which midi channel we want to use? (1-16)
  U16 playMode;			//play once or in loop
  U16 playState;		//default play mode: STOPPED or playing
  U32 eventPoolSize; 		//how big memory pool we want for midi events in the beginning
  U32 eventDataLaSize;   	//how big linear data allocator we want to have in the beginning for event data we want to have? 
  S32 midiBufferSize;    	//sets internal midi buffer to larger one than the system one
  S32 midiConnectionTimeOut;    //sets timeout in seconds for handshake connection 
  BOOL handshakeModeEnabled; 	//set to true if your module is connected to computers midi in
  BOOL streamed;		//stream everything from raw, not preprocessed files or preload everything to memory? [not implemented]
  BOOL useLZO;			//use/not use compression (slower loading time, data is depacked during runtime, can save some memory)[not implemented]
} tAmidiConfig;

S32 saveConfig(const U8 *config);
S32 loadConfig(const U8 *config);
void setConfig(tAmidiConfig *newConfig);
void setDefaultConfig();
const tAmidiConfig *getConfig();

#endif
