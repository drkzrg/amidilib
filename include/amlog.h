
#ifndef AMLOG_H_
#define AMLOG_H_

/**  Copyright 2007-2021 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#include "vartypes.h"
#include "core/amprintf.h"

#ifdef DEBUG_BUILD
#define DEBUG
#define _DEBUG
#endif

#if defined(DEBUG_SERIAL_OUTPUT_ONLY)

void serialLog(const char *mes,...);
#define amTrace(...) (serialLog(__VA_ARGS__))

#elif (defined(DEBUG)||defined(DEBUG_SERIAL_OUTPUT) && defined(DEBUG_FILE_OUTPUT))

void logd(const char *mes,...);
#define amTrace(...) (logd(__VA_ARGS__))

#else
#define amTrace(...) ((void)0)
#endif


void initDebug(const char *pFilename);
void deinitDebug(void);

#endif
