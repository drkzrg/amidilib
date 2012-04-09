
#include <amidilib.h>
#include "config.h"
#include <fmio.h>

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
static const U8 handshakeCommunicationEnabledTag[]={"handshakeEnabled"};
static const U8 streamedTag[]={"streamingEnabled"};
static const U8 lzoCompressionTag[]={"lzoDecompressionEnabled"};

S32 parseConfig (const U8* pData);

S32 saveConfig(const U8 *config){
  //save configuration state to file
  //prepare data
  //open file
  //slap data
  //close file
  //return 0 if ok
  return 0;
}

S32 loadConfig(const U8 *config){
//check if config file exists
//if not exit else parse it and se config
void *cfgData=0;
U32 cfgLen=0;
 
  cfgData=loadFile(config,PREFER_TT,&cfgLen);
  
  if(cfgData!=0){ 
    if(parseConfig(cfgData)>0){
      //parsed ok
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
  #ifndef PORTABLE
  configuration.midiBufferSize=MIDI_BUFFER_SIZE; //it's atari specific
  #endif
  configuration.midiConnectionTimeOut=5;	//5s by default
  configuration.handshakeModeEnabled=FALSE;
  configuration.streamed=FALSE;	
  configuration.useLZO=FALSE;		
}

//copies config
void setConfig(tAmidiConfig *newConfig){
  configuration.version=CONFIG_VERSION;		
  configuration.configSize=sizeof(tAmidiConfig);	
  configuration.connectedDeviceType=newConfig->connectedDeviceType;	
  configuration.midiChannel=newConfig->midiChannel;	
  configuration.playMode=newConfig->playMode;		
  configuration.playState=newConfig->playState;		
  #ifndef PORTABLE
  configuration.midiBufferSize=newConfig->midiBufferSize; 
  #endif
  configuration.midiConnectionTimeOut=newConfig->midiConnectionTimeOut;
  configuration.handshakeModeEnabled=newConfig->handshakeModeEnabled;
  configuration.streamed=newConfig->streamed;		
  configuration.useLZO=newConfig->useLZO;		
}

const tAmidiConfig *getConfig(){
  return &configuration;
}

S32 parseConfig(const U8* pData){
  setDefaultConfig();
  return 0;
}
