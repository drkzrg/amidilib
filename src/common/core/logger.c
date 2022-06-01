
/**  Copyright 2007-2021 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#include <stdarg.h>
#include <mint/osbind.h>

#ifdef ENABLE_GEMDOS_IO
#include "gemdosio.h"
#include <mint/ostruct.h>
#else
#include <stdlib.h>
#include <string.h>
#endif

#include "core/assert.h"
#include "core/logger.h"
#include "core/amprintf.h"

#include "memory/memory.h"

eTraceLevel globalTraceLevel;

static const eTraceLevel defaultTraceLevel = DEFAULT_LOG_LEVEL;

// Atari serial output
// TODO: console output to RAWCON(2(CON)->5(RAWCON))
// 1 - serial,
// 6 - st compatible serial (Modem 1, works with Hatari),
// 7 - scc (modem 2, doesn't work under Hatari)

static const uint8 SERIAL_OUTPUT_CHANNEL=7;

#if DEBUG_OUTPUT_TO_FILE

#ifdef ENABLE_GEMDOS_IO
int16 ofp = 0;
#else
FILE* ofp = 0;
#endif

static const char defaultLogName[] = "amidi.log";
#endif

#if (DEBUG_OUTPUT_TO_FILE || DEBUG_OUTPUT_TO_CONSOLE || DEBUG_OUTPUT_TO_DEBUGGER || DEBUG_OUTPUT_TO_SERIAL)

#if DEBUG_OUTPUT_TO_DEBUGGER
// TODO: Hatari output
#endif

static const char* arDebugLevelString[TL_ERR_NUM] =
{
    "",
    "[INFO]",
    "[MESSAGE]",
    "[WARNING]",
    "[ERROR]",
    "[FATAL ERROR]",
    ""
};

#define MIN_SRCPATHLEN 32
#define DEBUG_TRACE_FMT "\n%s[source]:(l:%u)%s\n# "

char outputTraceBuffer[DEBUG_OUTPUT_BUFFER_SIZE] = {0};

void logOutputTraceSimple(const eTraceLevel level, const char* const message, ...)
{
    if (level < globalTraceLevel) return;

#if (DEBUG_OUTPUT_TO_CONSOLE||DEBUG_OUTPUT_TO_FILE||DEBUG_OUTPUT_TO_DEBUGGER||DEBUG_OUTPUT_TO_SERIAL)
    int32_t sendCharsFlag=1;
    int32_t bytesSent=0;
#endif

    outputTraceBuffer[DEBUG_OUTPUT_BUFFER_SIZE-1]='\0';
 
    va_list va;
    va_start(va, message);
    amVsnprintf(outputTraceBuffer, (DEBUG_OUTPUT_BUFFER_SIZE - 1), message, va);
    va_end(va);

    const int32_t outBufferLength = strlen(outputTraceBuffer);
    AssertFatal((outBufferLength < (DEBUG_OUTPUT_BUFFER_SIZE - 1)), "Log buffer overrun!");

#if DEBUG_OUTPUT_TO_CONSOLE

#ifdef ENABLE_GEMDOS_IO
    {
        (void)Cconws(outputTraceBuffer);
    }
#else
    fprintf(stderr, "%s", outputTraceBuffer);
#endif

#endif

#if DEBUG_OUTPUT_TO_FILE

#ifdef ENABLE_GEMDOS_IO
    {
        const int32_t retVal = Fwrite(ofp, outBufferLength, outputTraceBuffer);

        if(retVal != outBufferLength)
        {
            amPrintf(NL "Not all data were saved to log file. (written: %d / expected: %d)]."NL,retVal, outBufferLength);
        }

    }
#else
    fprintf(ofp, outputTraceBuffer);
#endif

#endif

#if DEBUG_OUTPUT_TO_DEBUGGER
// TODO: Hatari output
#endif

#if DEBUG_OUTPUT_TO_SERIAL
    sendCharsFlag=1;
    bytesSent=0;

    while(sendCharsFlag)
    {
      if(Bcostat(SERIAL_OUTPUT_CHANNEL) == -1)
      {
        Bconout(SERIAL_OUTPUT_CHANNEL,(int16_t)outputTraceBuffer[bytesSent]);
        ++bytesSent;
      }

     if(outBufferLength==bytesSent)
     {
        sendCharsFlag=0;
     }
    }
#endif    

}

void logOutputTrace(const eTraceLevel level, const char * const sourceName, const size_t lineNb, const char * const message, ...)
{
    if (level < globalTraceLevel) return;

    outputTraceBuffer[DEBUG_OUTPUT_BUFFER_SIZE-1]='\0';

#if (DEBUG_OUTPUT_TO_CONSOLE||DEBUG_OUTPUT_TO_FILE||DEBUG_OUTPUT_TO_DEBUGGER||DEBUG_OUTPUT_TO_SERIAL)
    int32_t sendCharsFlag=1;
    int32_t bytesSent=0;
#endif

    const int32_t srcLen = strlen(sourceName);
    const char* truncSourcePathStart = (srcLen > MIN_SRCPATHLEN) ? (sourceName + srcLen - MIN_SRCPATHLEN) : sourceName;

    amSnprintf(outputTraceBuffer, DEBUG_OUTPUT_BUFFER_SIZE, DEBUG_TRACE_FMT, arDebugLevelString[level], (uint32_t)lineNb, truncSourcePathStart);

    int32_t outBufferLength = strlen(outputTraceBuffer);
    
    va_list va;
    va_start(va, message);
    amVsnprintf((outputTraceBuffer + outBufferLength), (DEBUG_OUTPUT_BUFFER_SIZE-1-outBufferLength), message, va);
    va_end(va);

    outBufferLength = strlen(outputTraceBuffer);
    AssertFatal((outBufferLength<(DEBUG_OUTPUT_BUFFER_SIZE-1)),"Log buffer overrun!");

#if DEBUG_OUTPUT_TO_CONSOLE
#ifdef ENABLE_GEMDOS_IO
    {
        (void)Cconws(outputTraceBuffer);
    }
#else
    fprintf(stderr, "%s", outputTraceBuffer);
#endif    
#endif

#if DEBUG_OUTPUT_TO_FILE

#ifdef ENABLE_GEMDOS_IO
    {
        const int32_t retVal = Fwrite(ofp, outBufferLength, outputTraceBuffer);

        if(retVal != outBufferLength)
        {
            amPrintf(NL "Not all data were saved to log file. (written: %d / expected: %d)]."NL,retVal, outBufferLength);
        }

    }
#else
    fprintf(ofp, outputTraceBuffer);
#endif    

#endif

#if DEBUG_OUTPUT_TO_DEBUGGER
// TODO: Hatari output
#endif

#if DEBUG_OUTPUT_TO_SERIAL
    sendCharsFlag=1;
    bytesSent=0;

    while(sendCharsFlag)
    {
      if(Bcostat(SERIAL_OUTPUT_CHANNEL) == -1)
      {
        Bconout(SERIAL_OUTPUT_CHANNEL,(int16_t)outputTraceBuffer[bytesSent]);
        ++bytesSent;
      }

     if(outBufferLength==bytesSent)
     {
        sendCharsFlag=0;
     }
    }
#endif    

}

#else

void logOutputTrace(const eTraceLevel level, const char* const sourceName, const size_t lineNb, const char* const message, ...)
{}

void logOutputTraceSimple(const eTraceLevel level, const char* const message, ...)
{}

#endif

void setLogTraceLevel(const eTraceLevel level) 
{
    globalTraceLevel = level;
}

void initLogger(const char *const logName)
{
    globalTraceLevel = defaultTraceLevel;

#if DEBUG_OUTPUT_TO_SERIAL
    Bconmap(SERIAL_OUTPUT_CHANNEL);
#endif    

#if DEBUG_OUTPUT_TO_FILE

#ifdef ENABLE_GEMDOS_IO

    if(logName)
        ofp = Fcreate(logName, 0);
    else
        ofp = Fcreate(defaultLogName, 0);

    if(ofp<=0)
    {
        amPrintf("[GEMDOS] Error: %s"NL,getGemdosError(ofp));
        amPrintf("Couldn't create log file: %s"NL,(logName ? logName : defaultLogName ));
    }

#else    

  if(logName)
    ofp = fopen(logName, "w+");
  else
     ofp = fopen(defaultLogName, "w+");

  if (ofp == NULL) 
  {
    if (logName)
        fprintf(stderr, "Can't open file output: %s\n", logName);
    else
        fprintf(stderr, "Can't open file output: %s\n", defaultLogName);
  }
#endif

#endif
}

void deinitLogger(void) 
{
#if DEBUG_OUTPUT_TO_FILE

#ifdef ENABLE_GEMDOS_IO

    const int16 retVal = Fclose(ofp);

    if(retVal != GDOS_OK)
    {
      amPrintf("[GEMDOS] Error closing file handle : [%d] %s"NL, ofp, getGemdosError(retVal));
    }

#else    
  fflush(ofp);
  fclose(ofp);
#endif

#endif
}
