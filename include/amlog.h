#ifndef _DEBUG_H_
#define _DEBUG_H_

/**
    Copyright 2010-2013 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    See license.txt for licensing information.
*/

#include "c_vars.h"

#if defined(DEBUG_SERIAL_OUTPUT)
#define amTrace(...) (serialLog(__VA_ARGS__))
void serialLog(const char *mes,...);
#elif (defined(DEBUG) && !defined(DEBUG_SERIAL_OUTPUT))||defined(DEBUG_FILE_OUTPUT)
#define amTrace(...) (logd(__VA_ARGS__))
void logd(const char *mes,...);
#else
#define amTrace(...) ((void)0)
#endif


void initDebug(const char *pFilename);
void deinitDebug();

#endif
