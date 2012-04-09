
#include <amidilib.h>
#include "config.h"
#include <fmio.h>

//internal configuration
static tAmidiConfig configuration;

static const U16 CONFIG_VERSION =0x0001; 	//config version

//configuration tags
static const U8 versionTag[]={"ver"};
static const U8 connectedDeviceTag[]={"deviceType"};
static const U8 defaultMidiChannelTag[]={"defaultMidiChannel"};
static const U8 defaultPlayModeTag[]={"defaultPlayMode"};
static const U8 defaultPlayStateTag[]={"defaultPlayState"};
static const U8 defaultEventPoolSizeTag[]={"defaultEventPoolSize"};
static const U8 defaultEventDataLaSizeTag[]={"defaultEventDataLaSize"};   
static const U8 defaultMidiBufferSizeTag[]={"defaultMidiBufferSize"};
static const U8 streamedTag[]={"streamed"};
static const U8 lzoCompressionTag[]={"useLZO"};

U32 saveConfig(const U8 *config){
  //save configuration state to file
  
  return 0;
}

U32 loadConfig(const U8 *config){
//check if config file exists
//if not exit else parse it and se config
 return 0;
}

void setDefaultConfig(){
  configuration.version=CONFIG_VERSION;		
  configuration.configSize=sizeof(tAmidiConfig);	
  configuration.connectedDeviceType = DT_LA_SOUND_SOURCE_EXT; 	//default is CM32L output device with extra patches	
  configuration.defaultMidiChannel=1;	
  configuration.defaultPlayMode=S_PLAY_ONCE;	//play once or in loop
  configuration.defaultPlayState=PS_STOPPED;	//default play state: STOPPED or playing
  #ifndef PORTABLE
  configuration.defaultMidiBufferSize=MIDI_BUFFER_SIZE; 
  #endif
  configuration.streamed=FALSE;	
  configuration.useLZO=FALSE;		
}

//copies config
void setConfig(tAmidiConfig *newConfig){
  configuration.version=CONFIG_VERSION;		
  configuration.configSize=sizeof(tAmidiConfig);	
  configuration.connectedDeviceType=newConfig->connectedDeviceType;	
  configuration.defaultMidiChannel=newConfig->defaultMidiChannel;	
  configuration.defaultPlayMode=newConfig->defaultPlayMode;		
  configuration.defaultPlayState=newConfig->defaultPlayState;		
  #ifndef PORTABLE
  configuration.defaultMidiBufferSize=newConfig->defaultMidiBufferSize; 
  #endif
  configuration.streamed=newConfig->streamed;		
  configuration.useLZO=newConfig->useLZO;		
}

const tAmidiConfig *getConfig(){
  return &configuration;
}

