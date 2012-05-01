
/**  Copyright 2007-2012 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#include <amidilib.h>
#include <fmio.h>

#include "config.h"

//internal configuration
static tAmidiConfig configuration;

static const U16 CONFIG_VERSION =0x0001; 	//config version

//configuration tags
static const U8 versionTag[]={"ver"};
static const U8 connectedDeviceTag[]={"deviceType"};
static const U8 midiChannelTag[]={"midiChannel"};
static const U8 playModeTag[]={"defaultPlayMode"};
static const U8 playStateTag[]={"defaultPlayState"};
static const U8 eventPoolSizeTag[]={"eventPoolSize"};
static const U8 eventDataAllocatorSizeTag[]={"eventDataAllocatorSize"};   
#ifndef PORTABLE
static const U8 midiBufferSizeTag[]={"midiBufferSize"};
#endif
static const U8 midiConnectionTimeoutTag[]={"midiTimeout"};
static const U8 handshakeCommunicationEnabledTag[]={"handshakeEnabled"};
static const U8 streamedTag[]={"streamingEnabled"};
static const U8 lzoCompressionTag[]={"lzoDecompressionEnabled"};
static const U8 silenceThresholdTag[]={"silenceThreshold"};

#define CONFIG_SIZE 512		//should be sufficient		 

S32 parseConfig (const U8* pData);

S32 saveConfig(const U8 *configFileName){
  U8 configData[CONFIG_SIZE]; 
  configData[0]='\0';
  
  //prepare data
  U32 length = 0;
  
  length+=snprintf(configData + length, CONFIG_SIZE-length,"%s = %d\r\n", versionTag,configuration.version);
  length+=snprintf(configData + length, CONFIG_SIZE-length,"%s = %d\r\n", connectedDeviceTag,configuration.connectedDeviceType);
  length+=snprintf(configData + length, CONFIG_SIZE-length,"%s = %d\r\n", midiChannelTag,configuration.midiChannel);
  length+=snprintf(configData + length, CONFIG_SIZE-length,"%s = %d\r\n", playModeTag,configuration.playMode);
  length+=snprintf(configData + length, CONFIG_SIZE-length,"%s = %d\r\n", playStateTag,configuration.playState);
  length+=snprintf(configData + length, CONFIG_SIZE-length,"%s = %d\r\n", eventPoolSizeTag,configuration.eventPoolSize);
  length+=snprintf(configData + length, CONFIG_SIZE-length,"%s = %d\r\n", eventDataAllocatorSizeTag,configuration.eventDataAllocatorSize);
  
  #ifndef PORTABLE
  length+=snprintf(configData + length, CONFIG_SIZE-length,"%s = %d\r\n", midiBufferSizeTag,configuration.midiBufferSize);
  #endif
 
  length+=snprintf(configData + length, CONFIG_SIZE-length,"%s = %d\r\n", midiConnectionTimeoutTag,configuration.midiConnectionTimeOut);
  length+=snprintf(configData + length, CONFIG_SIZE-length,"%s = %d\r\n", handshakeCommunicationEnabledTag,configuration.handshakeModeEnabled);
  length+=snprintf(configData + length, CONFIG_SIZE-length,"%s = %d\r\n", silenceThresholdTag,configuration.midiSilenceThreshold);
  length+=snprintf(configData + length, CONFIG_SIZE-length,"%s = %d\r\n", streamedTag,configuration.streamed);
  length+=snprintf(configData + length, CONFIG_SIZE-length,"%s = %d\r\n", lzoCompressionTag,configuration.useLZO);
  
  //save configuration state to file
  if(saveFile(configFileName,(void *)configData,length)<0){
    return -1L;
  }
  
  return 0;
}

S32 loadConfig(const U8 *configFileName){
//check if config file exists
//if not exit else parse it and set config
void *cfgData=0;
U32 cfgLen=0;
 
  cfgData=loadFile(configFileName,PREFER_TT,&cfgLen);
  
  if(cfgData!=0){ 
    if(parseConfig(cfgData)>0){
      //parsed ok
      setDefaultConfig(); //TODO: remove it when parsing will be working
    }else{
      //not ok reset to defaults
      setDefaultConfig();
    }
    
    amFree(&cfgData);
    return 0;
  }
  else return -1L; //fuck up!
}

void setDefaultConfig(){
  configuration.version=CONFIG_VERSION;		
  configuration.configSize=sizeof(tAmidiConfig);	
  configuration.connectedDeviceType = DT_LA_SOUND_SOURCE_EXT; 	//default is CM32L output device with extra patches	
  configuration.midiChannel = 1;	
  configuration.playMode = S_PLAY_ONCE;	//play once or in loop
  configuration.playState = PS_STOPPED;	//default play state: STOPPED or playing
  configuration.eventPoolSize=0;
  configuration.eventDataAllocatorSize=0;
  #ifndef PORTABLE
  configuration.midiBufferSize=MIDI_BUFFER_SIZE; //it's atari specific
  #endif
  configuration.midiConnectionTimeOut=5;	//5s by default
  configuration.midiSilenceThreshold=20;	
  configuration.handshakeModeEnabled=FALSE;
  configuration.streamed=FALSE;	
  configuration.useLZO=FALSE;		
}

//copies config
void setGlobalConfig(tAmidiConfig *newConfig){
  configuration.version=CONFIG_VERSION;		
  configuration.configSize=sizeof(tAmidiConfig);	
  configuration.connectedDeviceType=newConfig->connectedDeviceType;	
  configuration.midiChannel=newConfig->midiChannel;	
  configuration.playMode=newConfig->playMode;		
  configuration.playState=newConfig->playState;		
  configuration.eventPoolSize=newConfig->eventPoolSize;
  configuration.eventDataAllocatorSize=newConfig->eventDataAllocatorSize;
  
  #ifndef PORTABLE
  configuration.midiBufferSize=newConfig->midiBufferSize; 
  #endif
  configuration.midiConnectionTimeOut=newConfig->midiConnectionTimeOut;
  configuration.midiSilenceThreshold=newConfig->midiSilenceThreshold;
  configuration.handshakeModeEnabled=newConfig->handshakeModeEnabled;
  configuration.streamed=newConfig->streamed;		
  configuration.useLZO=newConfig->useLZO;		
}

const tAmidiConfig *getGlobalConfig(){
  return &configuration;
}

S32 parseConfig(const U8* pData){
  setDefaultConfig();
  return 0;
}

