
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <mint/osbind.h>

#ifdef ENABLE_GEMDOS_IO
#include <mint/ostruct.h>
#endif

#include "amlog.h"

// debugging OUTPUT settings
#define OUTPUT_BUFFER_SIZE 1024  // output buffer size

#if defined(DEBUG_FILE_OUTPUT)

#ifdef ENABLE_GEMDOS_IO
static S32 fh=0;
#else
static FILE *ofp=0;
#endif

#endif

#define DEBUG_LOG "LOG.TXT"

//todo: console output to RAWCON(2(CON)->5(RAWCON))
// 1 - serial,
// 6 - st compatible serial (Modem 1, works with hatari),
// 7 - scc (modem 2, doesn't work under hatari)

#define SERIAL_OUTPUT_CHANNEL	7

#if (defined(DEBUG)||defined(DEBUG_BUILD)||defined(DEBUG_SERIAL_OUTPUT)||defined(DEBUG_FILE_OUTPUT))
static char buffer[OUTPUT_BUFFER_SIZE];
#endif

#if (defined(DEBUG_BUILD)||defined(DEBUG_SERIAL_OUTPUT)||defined(DEBUG_FILE_OUTPUT))
void logd(const char *mes,...){

#if (defined(DEBUG_SERIAL_OUTPUT)||defined(DEBUG_CONSOLE_OUTPUT))
    int bBytesNotSent=1;
    int bytesSent=0;
    int len=0;
#endif

    va_list va;
    va_start(va,mes);
    vsprintf(buffer,(const char *)mes,va);
    va_end(va);

#if (defined(DEBUG_CONSOLE_OUTPUT)||defined(DEBUG_SERIAL_OUTPUT))
    len=strlen(buffer);
#endif

#ifdef DEBUG_CONSOLE_OUTPUT
    fprintf(stderr,buffer);
#endif

#ifdef DEBUG_SERIAL_OUTPUT
    while(bBytesNotSent){
      if( Bcostat(SERIAL_OUTPUT_CHANNEL) == -1 ){
        Bconout(SERIAL_OUTPUT_CHANNEL,(short int)buffer[bytesSent] );
        ++bytesSent;
      }
     if((len)==bytesSent)bBytesNotSent=0;
    }
#endif

#ifdef DEBUG_FILE_OUTPUT
    fprintf(ofp,buffer);
     fflush(ofp);
#endif
 return;
}
#endif

#ifdef DEBUG_SERIAL_OUTPUT_ONLY
void serialLog(const char *mes,...){
  int bSerialNotSent=1;
  int bytesSent=0;

  va_list va;
  va_start(va,mes);
  vsprintf(buffer,(const char *)mes,va);
  va_end(va);

  int len=strlen(buffer);

  while(bSerialNotSent){
      if( Bcostat(SERIAL_OUTPUT_CHANNEL) == -1 ){
        Bconout(SERIAL_OUTPUT_CHANNEL,(short int) buffer[bytesSent] );
        ++bytesSent;
      }

     if((len)==bytesSent) bSerialNotSent=0;
  }
}
#endif

void initDebug(const char *pFilename){

#ifdef DEBUG_FILE_OUTPUT

#ifdef ENABLE_GEMDOS_IO
    fh=FOpen(pFilename,FO_WRITE);

    if(fh<0){
        fprintf(stderr,"Can't init file output: %s\n",DEBUG_LOG);
    }

#else
    ofp=NULL;
    ofp=fopen(pFilename,"w");

    if(ofp==NULL){
        fprintf(stderr,"Can't init file output: %s\n",DEBUG_LOG);
    }
#endif

#endif

#ifdef DEBUG_SERIAL_OUTPUT
    Bconmap(SERIAL_OUTPUT_CHANNEL);
#endif
 return;
}

void deinitDebug(){
#if defined(DEBUG_FILE_OUTPUT)

#ifdef ENABLE_GEMDOS_IO
    FClose(fh);
#else
    fflush(ofp);
    fclose(ofp);
#endif

#endif
 return;
}
