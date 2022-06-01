
/**  Copyright 2007-2021 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#if !AMIDILIB_USE_LIBC

#define NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS 0
#define NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS 0 
#define NANOPRINTF_USE_FLOAT_FORMAT_SPECIFIERS 0 
#define NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS 0 
#define NANOPRINTF_USE_BINARY_FORMAT_SPECIFIERS 0 
#define NANOPRINTF_USE_WRITEBACK_FORMAT_SPECIFIERS 0

#define NANOPRINTF_IMPLEMENTATION 

#include "nanoprintf.h"
#include "vartypes.h"
#include "core/logger.h"
#include <mint/osbind.h>

#if NDEBUG
char outputTraceBuffer[DEBUG_OUTPUT_BUFFER_SIZE];
#else
AM_EXTERN char outputTraceBuffer[DEBUG_OUTPUT_BUFFER_SIZE];
#endif

int amCustomPrintf(const char* format, ...)
{

#ifndef SUPRESS_CON_OUTPUT
  
  outputTraceBuffer[DEBUG_OUTPUT_BUFFER_SIZE-1]='\0';

  va_list val;
  va_start(val, format);
  int const rv = npf_vsnprintf(&outputTraceBuffer[0],DEBUG_OUTPUT_BUFFER_SIZE-1,format,val);
  va_end(val);

  AssertMsg(rv>=0,"npf_vsnprintf() error");

  (void)Cconws(outputTraceBuffer);
#else
  int const rv = 0;
#endif

  return rv;
}

#endif

