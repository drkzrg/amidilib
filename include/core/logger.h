#ifndef LOGGER_H
#define LOGGER_H

/**  Copyright 2007-2022 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

// enable / disable output
#include "vartypes.h"

#if (_DEBUG || DEBUG)

#define DEBUG_OUTPUT_TO_FILE 0
#define DEFAULT_LOG_LEVEL TL_ERR_REPORT_ALL

#elif(NDEBUG)
#define DEBUG_OUTPUT_TO_CONSOLE 0
#define DEBUG_OUTPUT_TO_FILE 0
#define DEBUG_OUTPUT_TO_DEBUGGER 0
#define DEBUG_OUTPUT_TO_SERIAL 0
#undef DEFAULT_LOG_LEVEL
#define DEFAULT_LOG_LEVEL TL_ERR_FATAL
#else
#define DEBUG_OUTPUT_TO_CONSOLE 0
#define DEBUG_OUTPUT_TO_FILE 0
#define DEBUG_OUTPUT_TO_DEBUGGER 0
#define DEBUG_OUTPUT_TO_SERIAL 1
#undef DEFAULT_LOG_LEVEL
#define DEFAULT_LOG_LEVEL TL_ERR_REPORT_ALL
#endif

#define DEBUG_OUTPUT_BUFFER_SIZE (1024)  

// override setting if SUPRESS_CON_OUTPUT is defined
#ifdef SUPRESS_CON_OUTPUT

#ifdef DEBUG_OUTPUT_TO_CONSOLE
#undef DEBUG_OUTPUT_TO_CONSOLE
#define DEBUG_OUTPUT_TO_CONSOLE 0
#else
#define DEBUG_OUTPUT_TO_CONSOLE 0
#endif

#endif

typedef enum 
{
    TL_ERR_REPORT_ALL = 0,
    TL_ERR_INFO = 1,
    TL_ERR_MESSAGE = 2,
    TL_ERR_WARNING = 3,
    TL_ERR_ERROR = 4,
    TL_ERR_FATAL = 5,
    TL_ERR_REPORT_NONE = 6,
    TL_ERR_NUM
} eTraceLevel;

#define SID_FATAL(msg) TL_ERR_FATAL,__FILE__,__LINE__,msg
#define SID_ERROR(msg) TL_ERR_ERROR,__FILE__,__LINE__,msg
#define SID_WARNING(msg) TL_ERR_WARNING,__FILE__,__LINE__,msg
#define SID_MESSAGE(msg) TL_ERR_MESSAGE,__FILE__,__LINE__,msg
#define SID_INFO(msg) TL_ERR_INFO,__FILE__,__LINE__,msg

#if (DEBUG_OUTPUT_TO_FILE || DEBUG_OUTPUT_TO_CONSOLE|| DEBUG_OUTPUT_TO_DEBUGGER || DEBUG_OUTPUT_TO_SERIAL)

#define AM_MSG_FATAL(msg,...) logOutputTrace(SID_FATAL(msg),__VA_ARGS__)
#define AM_MSG_ERROR(msg,...) logOutputTrace(SID_ERROR(msg),__VA_ARGS__)
#define AM_MSG_INFO(msg,...) logOutputTrace(SID_INFO(msg),__VA_ARGS__)
#define AM_MSG(msg,...) logOutputTrace(SID_MESSAGE(msg),__VA_ARGS__)
#define AM_MSG_WARN(msg,...) logOutputTrace(SID_WARNING(msg),__VA_ARGS__)

#define amTrace(msg,...) logOutputTraceSimple(TL_ERR_MESSAGE,msg,__VA_ARGS__)

#define DbgTrace(SID,msg, ...) logOutputTrace(SID(msg),__VA_ARGS__)
#define DbgHalt() do {;} while (1)

void logOutputTrace(const eTraceLevel level, const char* const sourceName, const uint32_t lineNb, const char* const message, ...);
void logOutputTraceSimple(const eTraceLevel level, const char* const message, ...);
#else

#define AM_MSG_FATAL(msg,...) ((void)0)
#define AM_MSG_ERROR(msg,...) ((void)0)
#define AM_MSG_INFO(msg,...) ((void)0)
#define AM_MSG(msg,...) ((void)0)
#define AM_MSG_WARN(msg,...) ((void)0)

#define amTrace(msg,...) ((void)0)

#define DbgTrace(SID,msg, ...) ((void)0)
#define DbgHalt() ((void)0)

void logOutputTrace(const eTraceLevel level, const char* const sourceName, const uint32_t lineNb, const char* const message, ...);
void logOutputTraceSimple(const eTraceLevel level, const char* const message, ...);
#endif

void initLogger(const char* const logName);
void setTraceLevel(const eTraceLevel level);
void deinitLogger(void);

#endif
