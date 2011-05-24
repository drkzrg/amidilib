
/**  Copyright 2007-2010 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#ifndef __IFF_H__
#define __IFF_H__

#include	"c_vars.h"
#include <string.h>
#include <stdio.h>

typedef S8 ID[4];             /* 4 chars in ' ' through '~' */
#ifdef _MSC_VER
typedef struct {
  U32 chunkID;
  U32 chSize;  /* sizeof(chData) */
  }IFF_Chunk;

#else
typedef struct {
  U32 chunkID;
  U32 chSize;  /* sizeof(chData) */
  }__attribute__((packed)) IFF_Chunk;


#endif

/* IFF ID's*/
#define ID_FORM   	0x464F524D  /*('F','O','R','M')*/
#define ID_LIST   	0x4C495354  /*('L','I','S','T')*/
#define ID_PROP   	0x50524F50  /*('P','R','O','P')*/
#define ID_CAT    	0x43415400  /*('C','A','T',' ')*/
#define ID_FILLER 	0x00000000  /*(' ',' ',' ',' ')*/
#define ID_END_MARK 0x00000000 /* TODO: !!! not set, need checking */

/* returns next chunk header or ID_END_MARK - reading from file */
U32 getChunkHeader(void);

/**
 * processes IFF header
 *
 * @param pPtr memory pointer where IFF file is stored(the beginning),
 * @param startOffset - offset from which the next header has to be fetched
 * @return returns new offset to the next IFF chunk
 **/
U32 processIFFChunk(void *pPtr, U32 startOffset);
void processChunk(U32 chunkID,U32 ulSize);
U8 checkParity(U32 value);

#endif
