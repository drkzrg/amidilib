
#ifndef AMLOG_H_
#define AMLOG_H_

/**
    Copyright 2010-2019 Pawel Goralski
    
    See license.txt for licensing information.
*/

#include "c_vars.h"

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
void deinitDebug();

#endif
