
/**  Copyright 2007-2012 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#include <amidilib.h>
#include <fmio.h>
#include <string.h>
#include <stdlib.h>


#include "config.h"

//internal configuration
static tAmidiConfig configuration;

static const U16 CONFIG_VERSION =0x0001+1; 	//config version

static const U8 TRUE_TAG[] = "true";
static const U8 FALSE_TAG[] = "false";

//configuration tags
static const U8 versionTag[]={"ver"};
static const U8 connectedDeviceTag[]={"deviceType"};
static const U8 operationModeTag[]={"deviceOperationMode"};
static const U8 midiChannelTag[]={"midiChannel"}; 
static const U8 playModeTag[]={"defaultPlayMode"};
static const U8 playStateTag[]={"defaultPlayState"};
static const U8 eventPoolSizeTag[]={"eventPoolSize"};
static const U8 eventDataAllocatorSizeTag[]={"eventDataAllocatorSize"};   
#ifndef PORTABLE
static const U8 midiBufferSizeTag[]={"midiBufferSize"};
#endif
static const U8 midiConnectionTimeoutTag[]={"midiConnectionTimeout"};
static const U8 handshakeCommunicationEnabledTag[]={"handshakeEnabled"};
static const U8 streamedTag[]={"streamingEnabled"};
static const U8 lzoCompressionTag[]={"lzoDecompressionEnabled"};
static const U8 silenceThresholdTag[]={"silenceThreshold"};

#define CONFIG_SIZE 512		//should be sufficient		 

S32 parseConfig (const U8* pData, const tMEMSIZE bufferLenght);

S32 saveConfig(const U8 *configFileName){
  U8 configData[CONFIG_SIZE]; 
  configData[0]='\0';
  
  //prepare data
  U32 length = 0;
  
  length+=snprintf(configData + length, CONFIG_SIZE-length,"%s = %x\r\n", versionTag,configuration.version);
  length+=snprintf(configData + length, CONFIG_SIZE-length,"%s = %d\r\n", connectedDeviceTag,configuration.connectedDeviceType);
  length+=snprintf(configData + length, CONFIG_SIZE-length,"%s = %d\r\n", operationModeTag,configuration.operationMode);
  
  length+=snprintf(configData + length, CONFIG_SIZE-length,"%s = %d\r\n", midiChannelTag,configuration.midiChannel);
  length+=snprintf(configData + length, CONFIG_SIZE-length,"%s = %d\r\n", playModeTag,configuration.playMode);
  length+=snprintf(configData + length, CONFIG_SIZE-length,"%s = %d\r\n", playStateTag,configuration.playState);
  length+=snprintf(configData + length, CONFIG_SIZE-length,"%s = %d\r\n", eventPoolSizeTag,configuration.eventPoolSize);
  length+=snprintf(configData + length, CONFIG_SIZE-length,"%s = %d\r\n", eventDataAllocatorSizeTag,configuration.eventDataAllocatorSize);
  
  #ifndef PORTABLE
  length+=snprintf(configData + length, CONFIG_SIZE-length,"%s = %d\r\n", midiBufferSizeTag,configuration.midiBufferSize);
  #endif
 
  length+=snprintf(configData + length, CONFIG_SIZE-length,"%s = %d\r\n", midiConnectionTimeoutTag,configuration.midiConnectionTimeOut);
  length+=snprintf(configData + length, CONFIG_SIZE-length,"%s = %s\r\n", handshakeCommunicationEnabledTag,configuration.handshakeModeEnabled?TRUE_TAG:FALSE_TAG);
  length+=snprintf(configData + length, CONFIG_SIZE-length,"%s = %d\r\n", silenceThresholdTag,configuration.midiSilenceThreshold);
  length+=snprintf(configData + length, CONFIG_SIZE-length,"%s = %s\r\n", streamedTag,configuration.streamed?TRUE_TAG:FALSE_TAG);
  length+=snprintf(configData + length, CONFIG_SIZE-length,"%s = %s\r\n", lzoCompressionTag,configuration.useLZO?TRUE_TAG:FALSE_TAG);
  
  //save configuration state to file
  if(saveFile(configFileName,(void *)configData,length)>=0L){
    return 0L;
  }
  
  return -1L;
}

S32 loadConfig(const U8 *configFileName){
//check if config file exists
//if not exit else parse it and set config
void *cfgData=0;
U32 cfgLen=0;
 
  cfgData=loadFile(configFileName,PREFER_TT,&cfgLen);
  
  if(cfgData!=0){ 
    if(parseConfig(cfgData, cfgLen)<0){
      //not ok reset to defaults
      printf("Invalid configuration. Resetting to defaults.\n");
      setDefaultConfig();
    }
    
    amFree(&cfgData);
    return 0L;
  }
  else 
    return -1L; //fuck up!
}

void setConnectedDeviceType(eMidiDeviceType type){
  configuration.connectedDeviceType=type;
}

void setDefaultConfig(){
  configuration.version=CONFIG_VERSION;		
  configuration.configSize=sizeof(tAmidiConfig);	
  configuration.connectedDeviceType = DT_LA_SOUND_SOURCE_EXT; 	//default is CM32L output device with extra patches	
  configuration.operationMode=0;	
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
  configuration.operationMode=newConfig->operationMode;	
  
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

S32 parseConfig(const U8* pData, U32 bufferLenght){
  
  S32 iError=0;
  
  //config version 
  if(getUShortVal(versionTag,pData,bufferLenght,&configuration.version)<0){ 
    iError=-1;
  }else{
    // check version if ok then proceed if not throw error
    if(configuration.version!=CONFIG_VERSION){
        iError=-1;
    }
  }
  
  if(getUShortVal(connectedDeviceTag,pData,bufferLenght,&configuration.connectedDeviceType)<0){ 
   iError=-1;
  }
 
  if(getUShortVal(operationModeTag,pData,bufferLenght, &configuration.operationMode)<0){ 
   iError=-1;
  }
  
  if(getUShortVal(midiChannelTag,pData,bufferLenght,&configuration.midiChannel)<0){ 
   iError=-1;
  }
 
 if(getUShortVal(playModeTag,pData,bufferLenght,&configuration.playMode)<0){ 
   iError=-1;
  }
  
  if(getUShortVal(playStateTag,pData,bufferLenght,&configuration.playState)<0){ 
   iError=-1;
  }
 
  if(getUIntVal(eventPoolSizeTag,pData,bufferLenght,&configuration.eventPoolSize)<0){ 
   iError=-1;
  }
 
  if(getUIntVal(eventDataAllocatorSizeTag,pData,bufferLenght,&configuration.eventDataAllocatorSize)<0){ 
   iError=-1;
  }
 
 
  if(getIntVal(midiConnectionTimeoutTag,pData,bufferLenght,&configuration.midiConnectionTimeOut)<0){ 
   iError=-1;
  }
  
  if(getIntVal(silenceThresholdTag,pData,bufferLenght,&configuration.midiSilenceThreshold)<0){ 
   iError=-1;
  }
  
  #ifndef PORTABLE
    if(getIntVal(midiBufferSizeTag,pData,bufferLenght,&configuration.midiBufferSize)<0){ 
      iError=-1;
    }
  #endif
  
  
  if(getBoolVal(handshakeCommunicationEnabledTag,pData,bufferLenght,&configuration.handshakeModeEnabled)<0){ 
      iError=-1;
  }
  
  if(getBoolVal(streamedTag,pData,bufferLenght,&configuration.streamed)<0){ 
      iError=-1;
  }
  
  if(getBoolVal(lzoCompressionTag,pData,bufferLenght,&configuration.useLZO)<0){ 
      iError=-1;
  }
  
  return iError;
}


//helper functions
S32 getBoolVal(const U8* tagName, const U8 *data, const tMEMSIZE bufferLenght, BOOL *val){
  
  U8 *substrPtr=(U8 *) strstr((const char*)data, (const char*) tagName );
   
  if(substrPtr) {
      
       U8 *rval =(U8 *)strchr((data + (tMEMSIZE)(substrPtr - data)),'=');
	
       if(rval!=NULL){
	 rval++;
	 rval++;
	 
	if(strncmp(rval,TRUE_TAG,4)==0){
	  *val=TRUE;
	  
	   return 0; 
	}else if(strncmp(rval,FALSE_TAG,4)==0){
	  *val=FALSE;
	 
	  return 0; 
	}else{
	  amTrace("entry '%s' was not found\n", tagName); return -1;
	  return -1;
	}  
	
       }
  
      amTrace("entry '%s' was not found,\n", tagName); 
    return -1;
	  
  }
}

S32 getUIntVal(const U8* tagName, const U8 *data, const tMEMSIZE bufferLenght, U32 *val){
  U8 *substrPtr=(U8 *) strstr((const char*)data, (const char*) tagName );
  
  if(substrPtr) {
       
	
	U8 *rval =(U8 *)strchr((data + (tMEMSIZE)(substrPtr - data)),'=');
	
	if(rval!=NULL){
	 rval++;
	 rval++;
	 
	 *val=(U32)strtol(rval,(char **)NULL, 10);
	
	}else{
	  return -1;
	}
	
    } else {
        amTrace("entry '%s' was not found,\n", tagName);  return -1;
    }
  
  return -1;
}

S32 getIntVal(const U8* tagName, const U8 *data, const tMEMSIZE bufferLenght, S32 *val){
  U8 *substrPtr=(U8 *) strstr((const char*)data, (const char*) tagName );
  
  if(substrPtr) {
       
	U8 *rval =(U8 *)strchr((data + (tMEMSIZE)(substrPtr - data)),'=');
	
	if(rval!=NULL){
	 rval++;
	 rval++;
	 
	*val=(S32)strtol(rval,(char **)NULL, 10);
	
	 return 0; 
	}else{
	  return -1;
	}
    } else {
       amTrace("entry '%s' was not found,\n", tagName);  return -1;
    }
  
  return -1;
}

S32 getUShortVal(const U8* tagName, const U8 *data, const tMEMSIZE bufferLenght, U16 *val){
  
  U8 *substrPtr=(U8 *) strstr((const char*)data, (const char*) tagName );
  
  if(substrPtr) {
      
	U8 *rval =(U8 *)strchr((data + (tMEMSIZE)(substrPtr - data)),'=');
	
	if(rval!=NULL){
	 rval++;
	 rval++;
	 
	 *val=(U16)strtol(rval,(char **)NULL, 10);
	
	 return 0; 
	}else{
	  return -1;
	}
    } else {
       amTrace("entry '%s' was not found,\n", tagName); return -1;
    }
  
  return -1;
}

S32 getShortVal(const U8* tagName, const U8 *data, const tMEMSIZE bufferLenght, S16 *val){
  
  U8 *substrPtr=(U8 *)strstr((const char*)data, (const char*) tagName );
  
  if(substrPtr) {
        
	U8 *rval =(U8 *)strchr((data + (tMEMSIZE)(substrPtr - data)),'=');
	
	if(rval!=NULL){
	  rval++;
	  rval++;
	  
	  *val = (S16)strtol(rval,(char **)NULL, 10);
	
	 return 0; 
	}else{
	  return -1;
	}
	
    } else {
        amTrace("config entry '%s' was not found,\n", tagName); return -1;
    }
  
  return -1;
}



