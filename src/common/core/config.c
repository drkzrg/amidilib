
/**  Copyright 2007-2020 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/


#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <amidilib.h>
#include <fmio.h>
#include "config.h"

//internal configuration
static tAmidiConfig configuration;

static const uint16 CONFIG_VERSION = 4; 	// config version, changed default event memory pools for events
                                        // +1 removed EOT threshold, trackstate on bitfield, removed LZO from config
static const uint8 TRUE_TAG[] = "true";
static const uint8 FALSE_TAG[] = "false";

//configuration tags
static const uint8 versionTag[]={"ver"};
static const uint8 connectedDeviceTag[]={"deviceType"};
static const uint8 operationModeTag[]={"deviceOperationMode"};
static const uint8 midiChannelTag[]={"midiChannel"}; 
static const uint8 initialTrackStateTag[]={"defaultInitialTrackState"};
static const uint8 playStateTag[]={"defaultPlayState"};

#ifdef EVENT_LINEAR_BUFFER
static const uint8 eventPoolSizeTag[]={"eventPoolSize"};
static const uint8 eventDataAllocatorSizeTag[]={"eventDataAllocatorSize"};   
#endif

static const uint8 midiBufferSizeTag[]={"midiBufferSize"};
static const uint8 midiConnectionTimeoutTag[]={"midiConnectionTimeout"};
static const uint8 handshakeCommunicationEnabledTag[]={"handshakeEnabled"};
static const uint8 streamedTag[]={"streamingEnabled"};
static const uint8 lzoCompressionTag[]={"lzoDecompressionEnabled"};

// default values
#ifdef EVENT_LINEAR_BUFFER
static const uint32 DEFAULT_EVENT_POOL_SIZE =  24000UL; //nb of events
static const uint32 DEFAULT_EVENT_ALLOC_SIZE = 32UL;   //event size in bytes
#endif

static const int32 DEFAULT_MIDI_BUFFER_SIZE = MIDI_SENDBUFFER_SIZE; 	    	    // default operation mode (not used yet)

static const uint16 DEFAULT_CONNECTED_DEVICE_TYPE = DT_LA_SOUND_SOURCE_EXT; 	    // default connected device
static const uint16 DEFAULT_MIDI_CHANNEL = 1;                                      // default midi channel
static const uint16 DEFAULT_OP_MODE = 0;                                           // default operation mode //TODO: make it on strings / human readable
static const uint16 DEFAULT_TRACK_STATE = TM_PLAY_ONCE;
static const uint16 DEFAULT_MIDI_CONNECTION_TIMEOUT = 5;                           // external midi module connection timeout
static const bool DEFAULT_HANDSHAKEMODE_ENABLED = FALSE;
static const bool DEFAULT_USE_STREAMING = FALSE;                                // not used atm

static const uint16 CONFIG_SIZE = 512;		    //should be sufficient		 


int32 parseConfig (const uint8* pData, const MemSize bufferLenght);

int32 saveConfig(const uint8 *configFileName){
  uint8 configData[CONFIG_SIZE]; 
  configData[0]='\0';
  
  //prepare data
  uint32 length = 0;
  
  length+=snprintf(configData + length, CONFIG_SIZE-length,"%s = %x\r\n", versionTag,configuration.version);
  length+=snprintf(configData + length, CONFIG_SIZE-length,"%s = %d\r\n", connectedDeviceTag,configuration.connectedDeviceType);
  length+=snprintf(configData + length, CONFIG_SIZE-length,"%s = %d\r\n", operationModeTag,configuration.operationMode);
  
  length+=snprintf(configData + length, CONFIG_SIZE-length,"%s = %d\r\n", midiChannelTag,configuration.midiChannel);
  length+=snprintf(configData + length, CONFIG_SIZE-length,"%s = %d\r\n", initialTrackStateTag,configuration.initialTrackState);

#ifdef EVENT_LINEAR_BUFFER
  length+=snprintf(configData + length, CONFIG_SIZE-length,"%s = %lu\r\n", eventPoolSizeTag,configuration.eventPoolSize);
  length+=snprintf(configData + length, CONFIG_SIZE-length,"%s = %lu\r\n", eventDataAllocatorSizeTag,configuration.eventDataAllocatorSize);
#endif

  length+=snprintf(configData + length, CONFIG_SIZE-length,"%s = %ld\r\n", midiBufferSizeTag,configuration.midiBufferSize);
  length+=snprintf(configData + length, CONFIG_SIZE-length,"%s = %ld\r\n", midiConnectionTimeoutTag,configuration.midiConnectionTimeOut);
  length+=snprintf(configData + length, CONFIG_SIZE-length,"%s = %s\r\n", handshakeCommunicationEnabledTag,configuration.handshakeModeEnabled?TRUE_TAG:FALSE_TAG);
  length+=snprintf(configData + length, CONFIG_SIZE-length,"%s = %s\r\n", streamedTag,configuration.streamed?TRUE_TAG:FALSE_TAG);
  
  //save configuration state to file
  if(saveFile(configFileName,(void *)configData,length)>=0L){
    return 0L;
  }
  
  return -1L;
}

int32 loadConfig(const uint8 *configFileName){
//check if config file exists
//if not exit else parse it and set config
void *cfgData=0;
uint32 cfgLen=0;
 
  cfgData=loadFile(configFileName,PREFER_TT,&cfgLen);
  
  if(cfgData!=NULL){ 
    if(parseConfig(cfgData, cfgLen)<0){
      //not ok reset to defaults
      printf("Invalid configuration. Reset to defaults.\n");
      setDefaultConfig();
    }else{
      printf("Configuration loaded.\n");
    }
    
    gUserMemFree(cfgData,0);
    return 0L;
  }
  else{ 
    setDefaultConfig();
    printf("Configuration couldn't be loaded. Setting defaults...\n");
    return 0L; //fuck up!
  }
}

void setConnectedDeviceType(const eMidiDeviceType type){
  configuration.connectedDeviceType=type;
}

void setDefaultConfig(void)
{
  configuration.version=CONFIG_VERSION;		
  configuration.configSize=sizeof(tAmidiConfig);	
  configuration.connectedDeviceType = DEFAULT_CONNECTED_DEVICE_TYPE; 	//default is CM32L output device with extra patches	
  configuration.operationMode=DEFAULT_OP_MODE;	
  configuration.midiChannel = DEFAULT_MIDI_CHANNEL;	
  configuration.initialTrackState = DEFAULT_TRACK_STATE;				// play once or in loop

#ifdef EVENT_LINEAR_BUFFER
  configuration.eventPoolSize=DEFAULT_EVENT_POOL_SIZE;
  configuration.eventDataAllocatorSize=DEFAULT_EVENT_ALLOC_SIZE;
#endif

  configuration.midiBufferSize=DEFAULT_MIDI_BUFFER_SIZE;
  configuration.midiConnectionTimeOut=DEFAULT_MIDI_CONNECTION_TIMEOUT;	//5s by default
  configuration.handshakeModeEnabled=DEFAULT_HANDSHAKEMODE_ENABLED;
  configuration.streamed=DEFAULT_USE_STREAMING;	

}

//copies config
void setGlobalConfig(tAmidiConfig *newConfig)
{
  configuration.version=CONFIG_VERSION;		
  configuration.configSize=sizeof(tAmidiConfig);	
  configuration.connectedDeviceType=newConfig->connectedDeviceType;	
  configuration.operationMode=newConfig->operationMode;	
  
  configuration.midiChannel=newConfig->midiChannel;	
  configuration.initialTrackState=newConfig->initialTrackState;

#ifdef EVENT_LINEAR_BUFFER
  configuration.eventPoolSize=newConfig->eventPoolSize;
  configuration.eventDataAllocatorSize=newConfig->eventDataAllocatorSize;
#endif

  configuration.midiBufferSize=newConfig->midiBufferSize;
  configuration.midiConnectionTimeOut=newConfig->midiConnectionTimeOut;
  configuration.handshakeModeEnabled=newConfig->handshakeModeEnabled;
  configuration.streamed=newConfig->streamed;		

}

const tAmidiConfig *getGlobalConfig(void)
{
  return &configuration;
}

int32 parseConfig(const uint8* pData, const MemSize bufferLenght){
  
  int32 iError=0;
  
  //config version 
  iError=getUShortVal(versionTag,pData,bufferLenght,&configuration.version); 
   
   // check version if ok then proceed if not throw error
   if((iError>=0 && configuration.version!=CONFIG_VERSION)){
     printf("Wrong configuration version. Resetting to defaults.\n");  
     iError=-1;
     return (iError);
  }
  
  
  iError=getUShortVal(connectedDeviceTag,pData,bufferLenght,&configuration.connectedDeviceType);
  if(iError<0){
    configuration.connectedDeviceType=DEFAULT_CONNECTED_DEVICE_TYPE;
  }
  
  iError=getUShortVal(operationModeTag,pData,bufferLenght, &configuration.operationMode);
  if(iError<0){
    configuration.operationMode=DEFAULT_OP_MODE;
  }
  
  iError=getUShortVal(midiChannelTag,pData,bufferLenght,&configuration.midiChannel);
  if(iError<0){
    configuration.midiChannel=DEFAULT_MIDI_CHANNEL;
  }
  

  iError=getUShortVal(initialTrackStateTag,pData,bufferLenght,&configuration.initialTrackState);
  
  if(iError<0){
    configuration.initialTrackState=DEFAULT_TRACK_STATE;
  }
  
#ifdef EVENT_LINEAR_BUFFER
  iError=getUIntVal(eventPoolSizeTag,pData,bufferLenght,&configuration.eventPoolSize);
  
  if(iError<0){
    configuration.eventPoolSize=DEFAULT_EVENT_POOL_SIZE;
  }
  
  iError=getUIntVal(eventDataAllocatorSizeTag,pData,bufferLenght,&configuration.eventDataAllocatorSize);
  if(iError<0){
    configuration.eventDataAllocatorSize=DEFAULT_EVENT_ALLOC_SIZE;
  }
#endif

  iError=getIntVal(midiConnectionTimeoutTag,pData,bufferLenght,&configuration.midiConnectionTimeOut);
  
  if(iError<0){
    configuration.midiConnectionTimeOut=DEFAULT_MIDI_CONNECTION_TIMEOUT;
  }
  
  iError=getIntVal(midiBufferSizeTag,pData,bufferLenght,&configuration.midiBufferSize);
  if(iError<0){
    configuration.midiBufferSize=DEFAULT_MIDI_BUFFER_SIZE;
  }

  iError=getboolVal(handshakeCommunicationEnabledTag,pData,bufferLenght,&configuration.handshakeModeEnabled); 
  if(iError<0){
    configuration.handshakeModeEnabled=DEFAULT_HANDSHAKEMODE_ENABLED;
  }
  
  iError=getboolVal(streamedTag,pData,bufferLenght,&configuration.streamed);
  
  if(iError<0){
    configuration.streamed=DEFAULT_USE_STREAMING;
  }
  
  return iError;
}


// helper functions
int32 getboolVal(const uint8* tagName, const uint8 *data, const MemSize bufferLenght, bool *val){
  
  uint8 *substrPtr=(uint8 *)strstr((const char*)data, (const char*) tagName );
   
  if(substrPtr) {
      
       uint8 *rval =(uint8 *)strchr((data + (MemSize)(substrPtr - data)),'=');
	
       if(rval!=NULL){
            ++rval;
            ++rval;
	 
            if(strncmp(rval,TRUE_TAG,4)==0){
                *val=TRUE;
                return 0;
            }else if(strncmp(rval,FALSE_TAG,4)==0){
                *val=FALSE;
                return 0;
            }
       }
  }

  amTrace("entry '%s' was not found,\n", tagName);
  return -1;
}

int32 getUIntVal(const uint8* tagName, const uint8 *data, const MemSize bufferLenght, uint32 *val){
  uint8 *substrPtr=(uint8 *) strstr((const char*)data, (const char*) tagName );
  
  if(substrPtr) {
	uint8 *rval =(uint8 *)strchr((data + (MemSize)(substrPtr - data)),'=');
	
	if(rval!=NULL){
	 rval++;
	 rval++;
	 
	 *val=(uint32)strtol(rval,(char **)NULL, 10);
	 return 0; 
	}else{
	  return -1;
	}
	
    } else {
        amTrace("entry '%s' was not found,\n", tagName);  return -1;
    }
  
  return -1;
}

int32 getIntVal(const uint8* tagName, const uint8 *data, const MemSize bufferLenght, int32 *val){
  uint8 *substrPtr=(uint8 *) strstr((const char*)data, (const char*) tagName );
  
  if(substrPtr) {
       
	uint8 *rval =(uint8 *)strchr((data + (MemSize)(substrPtr - data)),'=');
	
	if(rval!=NULL){
	  rval++;
	  rval++;
	  *val=(int32)strtol(rval,(char **)NULL, 10);
	return 0; 
	}else{
	  return -1;
	}
    } 
    
  amTrace("entry '%s' was not found,\n", tagName);  
  return -1;
}

int32 getUShortVal(const uint8* tagName, const uint8 *data, const MemSize bufferLenght, uint16 *val){
  
  uint8 *substrPtr=(uint8 *) strstr((const char*)data, (const char*) tagName );
  
  if(substrPtr) {
      
	uint8 *rval =(uint8 *)strchr((data + (MemSize)(substrPtr - data)),'=');
	
	if(rval!=NULL){
	  rval++;
	  rval++;
	  *val=(uint16)strtol(rval,(char **)NULL, 10);
	  return 0; 
	}else{
	  return -1;
	}
    } 
  
  amTrace("entry '%s' was not found,\n", tagName); 
  return -1;
}

int32 getShortVal(const uint8* tagName, const uint8 *data, const MemSize bufferLenght, int16 *val){
  
  uint8 *substrPtr=(uint8 *)strstr((const char*)data, (const char*) tagName );
  
  if(substrPtr) {
	uint8 *rval =(uint8 *)strchr((data + (MemSize)(substrPtr - data)),'=');
	
	if(rval!=NULL){
	 rval++;
	 rval++;
	 *val = (int16)strtol(rval,(char **)NULL, 10);
	 return 0; 
	}else{
	  return -1;
	}
	
    } 
    
  amTrace("config entry '%s' was not found,\n", tagName); 
  return -1;
}



