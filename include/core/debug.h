#ifndef DEBUG_H
#define DEBUG_H

/**  Copyright 2011-21 Pawel Goralski
    This file is part of Nidhoggr engine.
    See license.txt for licensing information.
*/

#include "vartypes.h"

#if defined(DEBUG_SERIAL_OUTPUT)
#define DebugLog(...) (serialLog(__VA_ARGS__))
void serialLog(const char *mes, ...);
#elif ((defined(DEBUG) && !defined(DEBUG_SERIAL_OUTPUT)) && \
       defined(DEBUG_FILE_OUTPUT))
#define DebugLog(...) (logd(__VA_ARGS__))
void logd(const char *mes, ...);
#else
#define DebugLog(...) ((void)0)
#endif

//void initDebug(void);
//void deinitDebug(void);

#endif
