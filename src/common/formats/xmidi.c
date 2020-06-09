
/**  Copyright 2007-2020 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#include "fmio.h"
#include "memory.h"
#include "xmidi.h"
#include "memory/endian.h"

/* XMIDI contain midi events */
#include "midi.h"
#include <stdio.h>

/* 

XMIDI file contains single IFF "CAT " chunk of type XMID. The CAT chunk contains at least one XMIDI sequence, 
whose local chunks are stored within a "FORM" chunk of type XMID. As created by MIDIFORM, the XMIDI file may 
contain a chunk of type FORM XDIR which contains information about the file's collection of XMIDI sequences. 
The XDIR chunk is for the application's benefit, and is not currently required by the XMIDI drivers. 

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

// make ID
#define	MAKE_ID(a,b,c,d) ((uint32) (a)<<24 | (uint32) (b)<<16 | (uint32) (c)<<8 | (uint32) (d))

/* define XMIDI specific chunk defs */
#define ID_FORM_XDIR_INFO 	MAKE_ID('I','N','F','O') 
#define ID_FORM_XDIR 		MAKE_ID('X','D','I','R')
#define ID_FORM_XMID_TIMB 	MAKE_ID('T','I','M','B') 
#define ID_FORM_XMID_RBRN 	MAKE_ID('R','B','R','N') 
#define ID_FORM_XMID_EVNT 	MAKE_ID('E','V','N','T')
#define ID_CAT_XMID 		MAKE_ID('X','M','I','D') 
#define ID_FORM 			MAKE_ID('F','O','R','M')
#define ID_CAT 				MAKE_ID('C','A','T',' ')
#define ID_NULL  			MAKE_ID(' ',' ',' ',' ')

typedef int8 IFF_ID[4];

typedef struct IFFCHUNK {
	IFF_ID id;
	int32 size; 	
	uint8 *data; 	
} sIffChunk;

void *processIffChunks(sIffChunk *chunk, sSequence_t **ppCurSequence, int16 *iError);
void *processXmidiForm(void *dataStart,void *dataEnd, sSequence_t **ppCurSequence, int16 *iError);
void *processXmidiCat(void *dataStart,void *dataEnd, sSequence_t **ppCurSequence, int16 *iError);
void *processXmidiTimb(void *dataStart,void *dataEnd, sSequence_t **ppCurSequence, int16 *iError);
void *processXmidiRbrn(void *dataStart,void *dataEnd, sSequence_t **ppCurSequence, int16 *iError);
void *processXmidiEvnt(void *dataStart,void *dataEnd, sSequence_t **ppCurSequence, int16 *iError);

static INLINE int32 roundUp(const int32 val)
{
   if((val % 2)==0) return 0;
   return 1; // padded data
}

bool isValidXmidiData(void *midiData)
{
    bool isValid = false;

  	const sIffChunk *first_chunk = (sIffChunk *)midiData;
  	const uint32 iffId = *((uint32 *)first_chunk->id);

	if ( iffId == ID_FORM)  
	{
  		if( (uint32)first_chunk->data == ID_FORM_XDIR)
  		{
			const sIffChunk *infoChunk = (sIffChunk *)(((uintptr)&first_chunk->data) + sizeof(uint32));

  			// get nb of XMID FORMS
  			uint16 err=0;
  			uint16 nbOfXmidiFiles = (uint16)(((uintptr)infoChunk->data>>16)&0x0000FFFF);
  			nbOfXmidiFiles = ReadLE16(nbOfXmidiFiles);
  			
  			printf("XMidi files found: %d\n",nbOfXmidiFiles);
 			isValid = true;
   		} 
  
  		if(isValid == true)
  		{
    		// check next chunk for xmidi cat after xdir
    		const uint32 chunkSize = ReadBE32(first_chunk->size) + roundUp(first_chunk->size);
    		sIffChunk *nextChunk = (sIffChunk *)((uintptr)first_chunk + (uintptr)chunkSize + 8);

    		if(*((uint32 *)nextChunk->id) == ID_CAT)
    		{
      			if( (uint32)nextChunk->data != ID_CAT_XMID) isValid = false;
    		}

  		}
  
	} 
	else if(iffId == ID_CAT)
	{
  		if((uint32)first_chunk->data == ID_CAT_XMID)
  		{
    		isValid = true;
  		}
	}

 return isValid;
}

void *processIffChunks(sIffChunk *chunk, sSequence_t **ppCurSequence, int16 *iError)
{
	const uint32 id = *((uint32 *)chunk->id);
	const uint32 chunkSize = ReadBE32(chunk->size) + roundUp(chunk->size);

  	void *nextAddr = 0;

  	switch(id)
  	{
  		case ID_FORM:
  		{
 
  		} break;
 
  		case ID_FORM_XMID_TIMB:
  		{


  		} break;

  		case ID_FORM_XMID_RBRN:
  		{


  		} break;
  		case ID_FORM_XMID_EVNT:
  		{


  		} break;

		default: {
			// unknown id, skip it
		};		
  	};

	nextAddr = (void*)((uintptr)chunk->data + ((uintptr)chunkSize + 8));
	
	return nextAddr;
}

uint16 processXmidiData(void *data, const uint32 dataLength, sSequence_t **ppCurSequence)
{
	int32 chunkSize = 0;
	const void *endData = (void *)( (uintptr)data + (uintptr)dataLength );
	int16 err=0;

	while((data != endData) && (err==0))
	{
		sIffChunk *chunk = (sIffChunk *)data;
		data = processIffChunks(chunk, ppCurSequence, &err);
	}

	return 0;
}

void *processXmidiForm(void *dataStart,void *dataEnd, sSequence_t **ppCurSequence, int16 *iError)
{
	amTrace("FORM"); //XDIR or XMID
	return dataEnd;
}

void *processXmidiCat(void *dataStart,void *dataEnd, sSequence_t **ppCurSequence, int16 *iError)
{
	amTrace("CAT"); //XMID
	return dataEnd;
}

void *processXmidiTimb(void *dataStart,void *dataEnd, sSequence_t **ppCurSequence, int16 *iError)
{
	amTrace("TIMB");
	return dataEnd;
}

void *processXmidiRbrn(void *dataStart, void *dataEnd, sSequence_t **ppCurSequence, int16 *iError)
{
	amTrace("RBRN");
	return dataEnd;
}

/* XMIDI additional controllers */
#define C_CH_LOCK           0x6e        /* Channel Lock */
#define C_CH_LOCK_PROTECT   0x6f        /* Channel Lock Protect */
#define C_VOICE_PROTECT     0x70        /* Voice Protect */
#define C_TIMBRE_PROTECT    0x71        /* Timbre Protect */
#define C_PATCH_BANK_SELECT 0x72        /* Patch Bank Select */
#define C_IND_CTRL_PREFIX   0x73        /* Indirect Controller Prefix */
#define C_FOR_LOOP          0x74        /* For Loop Controller */
#define C_NEXT              0x75        /* Next/Break Loop Controller */
#define C_CLEAR_BAR_COUNT   0x76        /* Clear Beat/Bar Count */
#define C_CALL_TRIGGER      0x77        /* Callback Trigger */
#define C_SEQ_BRA_IDX       0x78        /* Sequence Branch Index */

void *processXmidiEvnt(void *dataStart,void *dataEnd, sSequence_t **ppCurSequence, int16 *iError)
{
	amTrace("EVNT");
	return dataEnd;
}



