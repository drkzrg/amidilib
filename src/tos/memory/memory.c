
/**  Copyright 2007-2010 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

// atari TOS version

#include "amlog.h"
#include "memory/memory.h"

#include <string.h>
#include <stdlib.h>


/**
 * gets amount of free preferred memory type (ST/TT RAM).
 * @param memFlag memory allocation preference flag
 * @return 0L - if no memory available, 0L< otherwise
 */
U32 getFreeMem(eMemoryFlag memFlag){
  void *pMem=0;
  pMem=(void *)Mxalloc( -1L, memFlag);
  return((U32)pMem);
}


#ifdef DEBUG_BUILD

void memoryCheck(void){
    U32 mem=0;
    amTrace((const U8*)"System memory check:\n");
	
    /* mem tst */
    mem=getFreeMem(ST_RAM);
    amTrace((const U8*)"ST-RAM: %u\n",(U32)mem);
	
    mem=getFreeMem(TT_RAM);
    amTrace((const U8*)"TT-RAM: %u\n",(U32)mem);
	
    mem=getFreeMem(PREFER_ST);
    amTrace((const U8*)"Prefered ST-RAM: %u\n",(U32)mem);
	
    mem=getFreeMem(PREFER_TT);
    amTrace((const U8*)"Prefered TT-RAM: %u\n",(U32)mem);
}
#endif


// writes a byte, and returns the buffer
U8* WriteByte(void* buf, S8 b){
    unsigned char* buffer = (unsigned char*)buf;
    *buffer = b;
    ++buffer;
    return buffer;
}

U8* WriteShort(void* b, U16 s){
    unsigned short* buffer = (unsigned short*)b;
    *buffer = s;
    ++buffer;
    return (unsigned char *)buffer;
}

U8* WriteInt(void* b, U32 i){
 unsigned int* buffer = (unsigned int*)b;
 *buffer = i;
 ++buffer;
 return (unsigned char *)buffer;
}

S32 UpdateBytesWritten(S32* bytes_written, S32 to_add, S32 max){
    *bytes_written += to_add;

    if(max==-1) return 1;

    if (max && *bytes_written > max){
        return 0;
    }
  return 1;
}
