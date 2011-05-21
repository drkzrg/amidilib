
/**  Copyright 2007-2010 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#ifndef __AMLOG_H__
#define __AMLOG_H__

#include <stdarg.h>
#include "c_vars.h"

/* small trick to get rid of logs/debug info in final build :D haxx0r! let the compiler do dirty work */
#if DEBUG_BUILD
#define amTrace am_log
#else
#define amTrace sizeof
#endif


/* variables for debug output to file */
#ifdef DEBUG_FILE_OUTPUT
FILE *ofp;
#endif

 BOOL CON_LOG;

 inline void am_initLog(const U8 *filename){
#ifdef DEBUG_CONSOLE_OUTPUT
  CON_LOG=TRUE;
#else
  CON_LOG=FALSE;
#endif

  
 /* if file output enabled open debug file */
#ifdef DEBUG_FILE_OUTPUT
 ofp = fopen((const char *)filename, "w");
 fflush(ofp);
 if (ofp == NULL) {
   fprintf(stderr,"Can't init debug file output to %s!\n",filename);
 }
 #endif
}

inline void am_deinitLog(){
  /* close debug file output */
  #ifdef DEBUG_FILE_OUTPUT
  fclose(ofp);
  #endif
}


inline void am_log(const U8 *mes,...){
static char buffer[256];

va_list va;
va_start(va, mes);
vsnprintf(buffer,sizeof(U8)*256,(const char *)mes,va);
va_end(va);   

#ifdef DEBUG_FILE_OUTPUT
    fprintf(ofp,buffer);
    fflush(ofp);
#endif

    
if(CON_LOG==TRUE) fprintf(stdout,buffer);


return;
}

#endif
