
/**  Copyright 2007-2010 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

//atari TOS version

#include "amlog.h"
#include "memory/memory.h"

#include <string.h>
#include <stdlib.h>

#ifdef DEBUG_MEM
static U32 g_memAlloc=0;
static U32 g_memAllocTT=0;
static U32 g_memDealloc=0;
#endif

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



void *amMemMove (void *pDest,void *pSrc,tMEMSIZE iSize){
 return memmove(pDest,pSrc,iSize);
}


void *amMemCpy (void *pDest, void *pSrc,tMEMSIZE iSize){
  U8 *pbDest=(U8 *)pDest;
  U8 *pbSrc=(U8 *)pSrc;
  
  if( (pbSrc<pbDest && (pbSrc + iSize)>pbDest ) || (pbDest<pbSrc && (pbDest +iSize) >pbSrc)){
    #ifdef DEBUG_MEM
      amTrace((const U8 *)"\tamMemCpy() overlaps. Using amMemMove()\n");
    #endif    
    return amMemMove(pDest,pSrc,iSize);
  }
  
  return memcpy(pDest,pSrc,iSize);
}

void *amMemSet ( void *pSrc,S32 iCharacter,tMEMSIZE iNum){
  void *pPtr=0;
  
  pPtr= memset(pSrc,iCharacter,iNum);
  
  #ifdef DEBUG_MEM
    if(pPtr!=pSrc) amTrace((const U8 *)"\tamMemSet() warning: returned pointers aren't equal!\n");
    else{
      amTrace((const U8 *)"\tamMemSet() memory: %p, %d x value written: %x!\n",pSrc,iNum,iCharacter);
    }
  #endif
  
  return pPtr;
}

int amMemCmp ( void *pSrc1, void *pSrc2, tMEMSIZE iNum){
  return memcmp(pSrc1,pSrc2,iNum);
}

void *amMemChr ( void *pSrc, S32 iCharacter, tMEMSIZE iNum){
  return memchr(pSrc,iCharacter,iNum);
}

void *amMallocEx(tMEMSIZE amount, U16 flag){
void *pMem=0;

#if (defined (FORCE_MALLOC))
  pMem = malloc(amount);
#else
  //TODO: check gemdos version and use Malloc() if needed for plain ST/e compatibility
  // make word alignment
  
  //TODO: add memory alignment build option on word, long boundary
  
  pMem= (void *)Mxalloc(amount,flag);
#endif

  #ifdef DEBUG_MEM
    if(pMem==0) {
      amTrace((const U8 *)"\tamMallocEx() Memory allocation error, returned NULL pointer! memory flag: %x\n",flag);      
    }else {
      g_memAllocTT++;
      amTrace((const U8 *)"\tamMallocEx() allocated %ld bytes at %p, mem flag: %x\n[AltAlloc nb: %d][memory left: %d]\n",amount,pMem,flag, g_memAllocTT, getFreeMem(flag) );
    }
  #endif
  
  return (void *)pMem;
}

void *amMalloc(tMEMSIZE amount){
void *pMem=0;

//TODO: add memory alignment build option on word, long boundary

#if (defined (FORCE_MALLOC))
  pMem= malloc(amount); 
#else
  pMem=(void *)Malloc(amount);
#endif
  
  #ifdef DEBUG_MEM
    if(pMem==0) {
      amTrace((const U8 *)"\tamMalloc() Memory allocation error,\n returned NULL pointer!!!!!\n");
    }
    else {
      g_memAlloc++;
      amTrace((const U8 *)"\tamMalloc() allocated %ld bytes at %p.\n[alloc nb: %d][memory left: %d]\n",amount,pMem,g_memAlloc,getFreeMem(PREFER_ST));
    }
  #endif
 return pMem;
}

void amFree(void **pPtr){   //TODO: make it as a macro

  #ifdef DEBUG_MEM
    if(*pPtr==0) {
      amTrace((const U8 *)"\tamFree() WARING: NULL pointer passed. \n");
    }
    else {
      g_memDealloc++;
      amTrace((const U8 *)"\tamFree() releasing memory at at %p [dealloc: %d]\n",*pPtr,g_memDealloc);
    }
  #endif
  
  #if (defined (FORCE_MALLOC))
  free(*pPtr); *pPtr=0;
  #else
    Mfree(*pPtr); *pPtr=0;
  #endif
}

void *amCalloc(tMEMSIZE nelements, tMEMSIZE elementSize){
  return calloc(nelements,elementSize);
}


void *amRealloc( void *pPtr, tMEMSIZE newSize){
 return realloc(pPtr,newSize);
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
