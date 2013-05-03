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
  U16 operationMode;		//we can have connected device with different operating modes(LA/GS/GM etc..) 
  U16 midiChannel;		//which midi channel we want to use? (1-16)
  U16 playMode;			//play once or in loop
  U16 playState;		//default play mode: STOPPED or playing [?]
  U32 eventPoolSize; 		//how big memory pool we want for midi events in the beginning
  U32 eventDataAllocatorSize;   //how big linear data allocator we want to have in the beginning for event data we want to have? 
  S32 midiBufferSize;    	//sets internal midi buffer to larger one than the system one
  S32 midiConnectionTimeOut;    //sets timeout in seconds for handshake connection 
  S32 midiSilenceThreshold;	//number of ticks for all channels after which end of track is detected
  BOOL handshakeModeEnabled; 	//set to true if your module is connected to computers midi in [not yet implemented]
  BOOL streamed;		//stream everything from raw, not preprocessed files or preload everything to memory? [not yet implemented]
  BOOL useLZO;			//use/not use compression (slower loading time, data is depacked during runtime, can save some memory)[not yet implemented]
} tAmidiConfig;

S32 saveConfig(const U8 *config);
S32 loadConfig(const U8 *config);
void setGlobalConfig(tAmidiConfig *newConfig);
void setDefaultConfig();

void setConnectedDeviceType(eMidiDeviceType type);
const tAmidiConfig *getGlobalConfig();

// parsing helper functions
S32 getBoolVal(const U8* tagName, const U8 *buffer, const tMEMSIZE bufferLenght, BOOL *val);
S32 getUIntVal(const U8* tagName, const U8 *buffer, const tMEMSIZE bufferLenght, U32 *val);
S32 getIntVal(const U8* tagName, const U8 *buffer, const tMEMSIZE bufferLenght, S32 *val);
S32 getUShortVal(const U8* tagName, const U8 *buffer, const tMEMSIZE bufferLenght, U16 *val);
S32 getShortVal(const U8* tagName, const U8 *buffer, const tMEMSIZE bufferLenght, S16 *val);

#endif
