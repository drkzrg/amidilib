
/**  Copyright 2007-2020 Pawel Goralski
    
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
uint32 getFreeMem(const eMemoryFlag memFlag){
  void *pMem=0;
  pMem=(void *)Mxalloc( -1L, memFlag);
  return((uint32)pMem);
}


#ifdef DEBUG_BUILD
static uint32 g_memAlloc=0;
static uint32 g_memAllocTT=0;
static uint32 g_memDealloc=0;

void memoryCheck(void){
    uint32 mem=0;
    amTrace((const uint8*)"System memory check:\n");
	
    /* mem tst */
    mem=getFreeMem(ST_RAM);
    amTrace((const uint8*)"ST-RAM: %u\n",(uint32)mem);
	
    mem=getFreeMem(TT_RAM);
    amTrace((const uint8*)"TT-RAM: %u\n",(uint32)mem);
	
    mem=getFreeMem(PREFER_ST);
    amTrace((const uint8*)"Prefered ST-RAM: %u\n",(uint32)mem);
	
    mem=getFreeMem(PREFER_TT);
    amTrace((const uint8*)"Prefered TT-RAM: %u\n",(uint32)mem);
}
#endif


// writes a byte, and returns the buffer
uint8* WriteByte(void* buf, int8 b){
    uint8* buffer = (unsigned char*)buf;
    *buffer = b;
    ++buffer;
    return buffer;
}

uint8* WriteShort(void* b, uint16 s){
    uint16* buffer = (unsigned short*)b;
    *buffer = s;
    ++buffer;
    return (uint8 *)buffer;
}

uint8* WriteInt(void* b, uint32 i){
 uint32* buffer = (uint32*)b;
 *buffer = i;
 ++buffer;
 return (uint8 *)buffer;
}

int32 UpdateBytesWritten(int32* bytes_written, int32 to_add, int32 max){
    *bytes_written += to_add;

    if(max==-1) return 1;

    if (max && *bytes_written > max){
        return 0;
    }
  return 1;
}
