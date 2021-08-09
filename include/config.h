/**  Copyright 2007-2021 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

// configuration managment, save/load settings from file
// here we can specify used device type, event memory pool size

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "vartypes.h"
#include "mdevtype.h"

typedef struct _amidiConfig{
  uint16 version;              //config version
  uint32 configSize;           //sizeof tAmidiConfig
  uint16 connectedDeviceType;	//as in eMidiDeviceTypes
  uint16 operationMode;		//we can have connected device with different operating modes(LA/GS/GM etc..) 
  uint16 midiChannel;          //which midi channel we want to use? (1-16)
  uint16 initialTrackState;	// initial play state bitfield is track stopped/playing/paused/loop/play once

#ifdef EVENT_LINEAR_BUFFER
  uint32 eventPoolSize;            //how big memory pool we want for midi events in the beginning
  uint32 eventDataAllocatorSize;   //how big linear data allocator we want to have in the beginning for event data we want to have? 
#endif

  int32 midiBufferSize;           //sets internal midi buffer to larger one than the system one
  int32 midiConnectionTimeOut;    //sets timeout in seconds for handshake connection 
  Bool handshakeModeEnabled; 	//set to true if your module is connected to computers midi in [not yet implemented]
  Bool streamed;                //stream everything from raw, not preprocessed files or preload everything to memory? [not yet implemented]
} tAmidiConfig;

int32 saveConfig(const uint8 *config);
int32 loadConfig(const uint8 *config);
void setGlobalConfig(tAmidiConfig *newConfig);
void setDefaultConfig(void);

void setConnectedDeviceType(const eMidiDeviceType type);
const tAmidiConfig *getGlobalConfig(void);

// parsing helper functions
int32 getboolVal(const uint8* tagName, const uint8 *buffer, const MemSize bufferLenght, bool *val);
int32 getUIntVal(const uint8* tagName, const uint8 *buffer, const MemSize bufferLenght, uint32 *val);
int32 getIntVal(const uint8* tagName, const uint8 *buffer, const MemSize bufferLenght, int32 *val);
int32 getUShortVal(const uint8* tagName, const uint8 *buffer, const MemSize bufferLenght, uint16 *val);
int32 getShortVal(const uint8* tagName, const uint8 *buffer, const MemSize bufferLenght, int16 *val);

#endif
