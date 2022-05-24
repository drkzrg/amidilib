
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

static void tosBiosConout(int c, void *wtf)
{
    Bconout(2,(uint16)c);
}

int amCustomPrintf(const char* format, ...)
{
  va_list val;
  va_start(val, format);
  int const rv = npf_vpprintf(&tosBiosConout,NULL,format,val);
  va_end(val);
  return rv;
}

#endif

