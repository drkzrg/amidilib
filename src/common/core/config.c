
/**  Copyright 2007-2019 Pawel Goralski
    
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

static const U16 CONFIG_VERSION = 4; 	// config version, changed default event memory pools for events
                                        // +1 removed EOT threshold, trackstate on bitfield, removed LZO from config
static const U8 TRUE_TAG[] = "true";
static const U8 FALSE_TAG[] = "false";

//configuration tags
static const U8 versionTag[]={"ver"};
static const U8 connectedDeviceTag[]={"deviceType"};
static const U8 operationModeTag[]={"deviceOperationMode"};
static const U8 midiChannelTag[]={"midiChannel"}; 
static const U8 initialTrackStateTag[]={"defaultInitialTrackState"};
static const U8 playStateTag[]={"defaultPlayState"};

#ifdef EVENT_LINEAR_BUFFER
static const U8 eventPoolSizeTag[]={"eventPoolSize"};
static const U8 eventDataAllocatorSizeTag[]={"eventDataAllocatorSize"};   
#endif

static const U8 midiBufferSizeTag[]={"midiBufferSize"};
static const U8 midiConnectionTimeoutTag[]={"midiConnectionTimeout"};
static const U8 handshakeCommunicationEnabledTag[]={"handshakeEnabled"};
static const U8 streamedTag[]={"streamingEnabled"};
static const U8 lzoCompressionTag[]={"lzoDecompressionEnabled"};

// default values
#ifdef EVENT_LINEAR_BUFFER
static const U32 DEFAULT_EVENT_POOL_SIZE =  12000UL; //nb of events
static const U32 DEFAULT_EVENT_ALLOC_SIZE = 32UL;   //event size in bytes
#endif

static const S32 DEFAULT_MIDI_BUFFER_SIZE = MIDI_SENDBUFFER_SIZE; 	    	    // default operation mode (not used yet)

static const U16 DEFAULT_CONNECTED_DEVICE_TYPE = DT_LA_SOUND_SOURCE_EXT; 	    // default connected device
static const U16 DEFAULT_MIDI_CHANNEL = 1;                                      // default midi channel
static const U16 DEFAULT_OP_MODE = 0;                                           // default operation mode //TODO: make it on strings / human readable
static const U16 DEFAULT_TRACK_STATE = TM_PLAY_ONCE;
static const U16 DEFAULT_MIDI_CONNECTION_TIMEOUT = 5;                           // external midi module connection timeout
static const BOOL DEFAULT_HANDSHAKEMODE_ENABLED = FALSE;
static const BOOL DEFAULT_USE_STREAMING = FALSE;                                // not used atm

static const U16 CONFIG_SIZE = 512;		    //should be sufficient		 


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

S32 loadConfig(const U8 *configFileName){
//check if config file exists
//if not exit else parse it and set config
void *cfgData=0;
U32 cfgLen=0;
 
  cfgData=loadFile(configFileName,PREFER_TT,&cfgLen);
  
  if(cfgData!=NULL){ 
    if(parseConfig(cfgData, cfgLen)<0){
      //not ok reset to defaults
      printf("Invalid configuration. Reset to defaults.\n");
      setDefaultConfig();
    }else{
      printf("Configuration loaded.\n");
    }
    
    amFree(cfgData);
    return 0L;
  }
  else{ 
    setDefaultConfig();
    printf("Configuration couldn't be loaded. Setting defaults...\n");
    return 0L; //fuck up!
  }
}

void setConnectedDeviceType(eMidiDeviceType type){
  configuration.connectedDeviceType=type;
}

void setDefaultConfig(){
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
void setGlobalConfig(tAmidiConfig *newConfig){
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

const tAmidiConfig *getGlobalConfig(){
  return &configuration;
}

S32 parseConfig(const U8* pData, const tMEMSIZE bufferLenght){
  
  S32 iError=0;
  
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

  iError=getBoolVal(handshakeCommunicationEnabledTag,pData,bufferLenght,&configuration.handshakeModeEnabled); 
  if(iError<0){
    configuration.handshakeModeEnabled=DEFAULT_HANDSHAKEMODE_ENABLED;
  }
  
  iError=getBoolVal(streamedTag,pData,bufferLenght,&configuration.streamed);
  
  if(iError<0){
    configuration.streamed=DEFAULT_USE_STREAMING;
  }
  
  return iError;
}


// helper functions
S32 getBoolVal(const U8* tagName, const U8 *data, const tMEMSIZE bufferLenght, BOOL *val){
  
  U8 *substrPtr=(U8 *)strstr((const char*)data, (const char*) tagName );
   
  if(substrPtr) {
      
       U8 *rval =(U8 *)strchr((data + (tMEMSIZE)(substrPtr - data)),'=');
	
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

S32 getUIntVal(const U8* tagName, const U8 *data, const tMEMSIZE bufferLenght, U32 *val){
  U8 *substrPtr=(U8 *) strstr((const char*)data, (const char*) tagName );
  
  if(substrPtr) {
	U8 *rval =(U8 *)strchr((data + (tMEMSIZE)(substrPtr - data)),'=');
	
	if(rval!=NULL){
	 rval++;
	 rval++;
	 
	 *val=(U32)strtol(rval,(char **)NULL, 10);
	 return 0; 
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
    } 
    
  amTrace("entry '%s' was not found,\n", tagName);  
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
    } 
  
  amTrace("entry '%s' was not found,\n", tagName); 
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
	
    } 
    
  amTrace("config entry '%s' was not found,\n", tagName); 
  return -1;
}



