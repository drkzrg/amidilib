
/**  Copyright 2007-2010 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#include "include/amlog.h"

//inits logs
extern inline void am_initLog();

extern inline void am_deinitLog();

/** Utility function sends text buffer to console and text log.
*   @param mes - null terminated string. Accepts additional formatting variables like sprintf/printf.
*/
extern inline void am_log(const U8 *mes,...);


