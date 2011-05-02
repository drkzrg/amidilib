
/**  Copyright 2007-2010 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#include <stdarg.h>
#include "include/amlog.h"

/* variables for debug output to file */
#ifdef DEBUG_FILE_OUTPUT
FILE *ofp;
#endif

BOOL CON_LOG;

void am_log(const U8 *mes,...){
static char buffer[256];

va_list va;
va_start(va, mes);
vsprintf(buffer,(const char *)mes,va);
va_end(va);   

if(CON_LOG==TRUE) fprintf(stdout,buffer);

#ifdef DEBUG_FILE_OUTPUT
    fprintf(ofp,buffer);
    fflush(ofp);
#endif

return;
}
