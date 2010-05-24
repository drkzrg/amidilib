/**  Copyright 2008, 2009 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.

    AMIDILIB is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    AMIDILIB is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with AMIDILIB.  If not, see <http://www.gnu.org/licenses/>.*/

#ifndef __IFF_H__
#define __IFF_H__

#include	"c_vars.h"
#include <string.h>
#include <stdio.h>

typedef S8 ID[4];             /* 4 chars in ' ' through '~' */

typedef struct {
  U32 chunkID;
  U32 chSize;  /* sizeof(chData) */
  } IFF_Chunk;


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
