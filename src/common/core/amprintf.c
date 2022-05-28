
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
#include <mint/osbind.h>
#include "vartypes.h"

#define OUTPUT_TEMP_BUFFER  1024

int amCustomPrintf(const char* format, ...)
{
  static char tempBuf[OUTPUT_TEMP_BUFFER] = {0};
  
  tempBuf[0]='\0';

  va_list val;
  va_start(val, format);
  int const rv = npf_vsnprintf(&tempBuf[0],OUTPUT_TEMP_BUFFER,format,val);
  va_end(val);

  AssertMsg(rv>=0,"npf_vsnprintf() error");

  (void)Cconws(tempBuf);

  return rv;
}

#endif

