
/**  Copyright 2007-2010 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#ifndef __AMLOG_H__
#define __AMLOG_H__

#include <stdarg.h>
#include "c_vars.h"

#ifndef _MSC_VER
#include <unistd.h>
#endif

/* small trick to get rid of logs/debug info in final build :D haxx0r! let the compiler do dirty work */
#if DEBUG_BUILD
#define amTrace am_log
#else
#define amTrace sizeof
#endif

//inits logs
void am_initLog();

void am_deinitLog();

/** Utility function sends text buffer to console and text log.
*   @param mes - null terminated string. Accepts additional formatting variables like sprintf/printf.
*/
void am_log(const U8 *mes,...);




#endif
