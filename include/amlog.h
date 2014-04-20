#ifndef _DEBUG_H_
#define _DEBUG_H_

/**
    Copyright 2010-2013 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    See license.txt for licensing information.
*/

#include "c_vars.h"

#ifdef DEBUG_BUILD
#define DEBUG
#endif

#ifdef DEBUG_SERIAL_OUTPUT_ONLY
#warning DEBUG serial output enabled
void serialLog(const char *mes,...);
#define amTrace(...) (serialLog(__VA_ARGS__))
#elif (defined(DEBUG)||defined(DEBUG_SERIAL_OUTPUT)&&defined(DEBUG_FILE_OUTPUT))
#warning general debug output enabled
void logd(const char *mes,...);
#define amTrace(...) (logd(__VA_ARGS__))
#else
#warning DEBUG output disabled
#define amTrace(...) ((void)0)
#endif


void initDebug(const char *pFilename);
void deinitDebug();

#endif
