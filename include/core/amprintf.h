
#ifndef AMPRINTF_H
#define AMPRINTF_H

/**  Copyright 2007-2021 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#include "vartypes.h"

#if AMIDILIB_USE_LIBC

#include <stdio.h>
#define amPrintf printf
#define amSnprintf snprintf
#define amVsnprintf vsnprintf
#define NL "\n"

#else

#include "nanoprintf.h"

#define amPrintf amCustomPrintf
#define amSnprintf npf_snprintf
#define amVsnprintf npf_vsnprintf

#define NL "\r\n"

int amCustomPrintf(const char* format, ...);

#endif

#endif
