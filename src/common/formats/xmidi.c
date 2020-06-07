
/**  Copyright 2007-2020 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#include "fmio.h"
#include "memory.h"
#include "xmidi.h"
#include "memory/endian.h"

/* XMIDI file contains single IFF "CAT " chunk of type XMID. The CAT chunk contains at least one XMIDI sequence, 
whose local chunks are stored within a "FORM" chunk of type XMID. As created by MIDIFORM, the XMIDI file may 
contain a chunk of type FORM XDIR which contains information about the file's collection of XMIDI sequences. 
The XDIR chunk is for the application's benefit, and is not currently required by the XMIDI drivers. */

/*
<len> signifies a 32-bit "big endian" chunk length, which includes neither itself nor its preceding 4-
character CAT , FORM, or local chunk name. 
Square brackets enclose optional chunks; ellipses are placed after the closing braces of chunks or data items which
may be repeated.

[ 
	FORM<len>XDIR 
	{ 
		INFO<len> UWORD # of FORMs XMID in file, 1-65535 
	} 
]

CAT <len>XMID 
{
	FORM<len>XMID 
	{ 
		[ TIMB<len> UWORD # of timbre list entries, 0-16384 
			{ 
				UBYTE patch number 0-127 UBYTE timbre bank 0-127 
			} 
			... 
		] 
		[ RBRN<len> UWORD # of branch point offsets, 0-127 
			{ 
				UWORD Sequence Branch Index controller value 0-127 ULONG controller offset from start of EVNT chunk 
			} 
			... 
		]
		
		EVNT<len> 
		{ 
			UBYTE interval count (if < 128) UBYTE <MIDI event> (if > 127)
		} 
		...
	} ...
}
*/

void *processXmidiForm(void *dataStart,void *dataEnd, sSequence_t **ppCurSequence, int16 *iError);
void *processXmidiList(void *dataStart,void *dataEnd, sSequence_t **ppCurSequence, int16 *iError);
void *processXmidiCat(void *dataStart,void *dataEnd, sSequence_t **ppCurSequence, int16 *iError);
void *processXmidiProp(void *dataStart,void *dataEnd, sSequence_t **ppCurSequence, int16 *iError);
void *processIffChunk(sIffChunk *chunk, sSequence_t **ppCurSequence, int16 *iError);

static INLINE int32 roundUp(const int32 val){
   if((val % 2)==0) return 0;
   return 1; // padded data
}

void *processIffChunk(sIffChunk *chunk, sSequence_t **ppCurSequence, int16 *iError)
{
	const uint32 id = (uint32)chunk->id;
	const uint32 chunkSize = ReadBE32(chunk->size) + roundUp(chunk->size);
  	
  	processIffChunk(chunk, ppCurSequence, iError);

	return (void*)((uintptr)chunk->data + (uintptr)chunkSize);
}


uint16 processXmidiData(void *data, const uint32 dataLength, sSequence_t **ppCurSequence)
{
	int32 chunkSize = 0;
	const void *endData = (void *)( (uintptr)data + (uintptr)dataLength );
	int16 err=0;

	while((data != endData) && (err==0))
	{
		sIffChunk *chunk = (sIffChunk *)data;
		data = processIffChunk(chunk, ppCurSequence, &err);
	}

	return 0;
}

void *processXmidiForm(void *dataStart,void *dataEnd, sSequence_t **ppCurSequence, int16 *iError)
{
	return dataEnd;
}
void *processXmidiList(void *dataStart,void *dataEnd, sSequence_t **ppCurSequence, int16 *iError)
{
	return dataEnd;
}

void *processXmidiCat(void *dataStart,void *dataEnd, sSequence_t **ppCurSequence, int16 *iError)
{
	return dataEnd;
}

void *processXmidiProp(void *dataStart,void *dataEnd, sSequence_t **ppCurSequence, int16 *iError)
{
	return dataEnd;
}



