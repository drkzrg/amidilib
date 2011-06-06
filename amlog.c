
/**  Copyright 2007-2010 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#include "include/amlog.h"

/* variables for debug output to file */
#ifdef DEBUG_FILE_OUTPUT
FILE *ofp;
#endif

static BOOL CON_LOG;

void am_initLog(const U8 *filename){

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

void am_deinitLog(void){
  /* close debug file output */
  #ifdef DEBUG_FILE_OUTPUT
  fclose(ofp);
  #endif
}


void am_log(const U8 *mes,...){
static char buffer[256]={0};

va_list va;
va_start(va, mes);

S32 iWritten=vsnprintf(buffer,sizeof(U8)*256-1,(const char *)mes,va);
if(iWritten==-1) fprintf(stdout,"vsnprintf() error\n");

va_end(va);   

#ifdef DEBUG_FILE_OUTPUT
    fprintf(ofp,buffer);
    fflush(ofp);
#endif

    
if(CON_LOG==TRUE) fprintf(stdout,buffer);


return;
}


