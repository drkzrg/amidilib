
/**  Copyright 2007-2012 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/
#include "include/fmio.h"


S32 saveFile(const U8 *szFileName, const void *memBlock, const U32 memBlockSize){

  return 0;  
}


void *loadFile(const U8 *szFileName, eMemoryFlag memFlag,  U32 *fileLenght){
U32 size = 0;
void * ret=NULL;

	FILE *f = fopen(szFileName, "rb");
	
	if (f == NULL) 	{
	  am_log((const U8 *)"loadFile(), Error during file %s open.",szFileName);
	  return NULL; 
	}
	
	fseek(f, 0, SEEK_END);
	size = ftell(f);
	fseek(f, 0, SEEK_SET);
	ret = (void *)amMallocEx(size*sizeof(U8),PREFER_TT);
	
	if (size != fread(ret, sizeof(U8), size*sizeof(U8), f)) { 
		amFree(&ret);
		 am_log((const U8 *)"loadFile(), Error during file %s read.",szFileName);
		return NULL;
	} 
    fclose(f);
  return ret;
}

