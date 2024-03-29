
/**  Copyright 2007-2021 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#ifdef ENABLE_GEMDOS_IO
#include "gemdosio.h"
#endif

#include "memory.h"
#include "xmidi.h"
#include "memory/endian.h"

/* XMIDI contain midi events */
#include "midi.h"

#include "core/amprintf.h"

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
		INFO<len> UWORD # of FORMs XMID in file, 1-65535 (little endian, because why not!)
	} 
]

CAT <len>XMID 
{
	FORM<len>XMID 
	{ 
		[ TIMB<len> UWORD # of timbre list entries, 0-16384 
			{ 
				UBYTE patch number 0-127 
				UBYTE timbre bank 0-127 
			} 
			... 
		] 
		[ RBRN<len> UWORD # of branch point offsets, 0-127 
			{ 
				UWORD Sequence Branch Index controller value 0-127 
				ULONG controller offset from start of EVNT chunk 
			} 
			... 
		]
		
		EVNT<len> 
		{ 
			UBYTE interval count (if < 128) 
			UBYTE <MIDI event> (if > 127)
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
#define ID_XMID 			MAKE_ID('X','M','I','D') 
#define ID_FORM 			MAKE_ID('F','O','R','M')
#define ID_CAT 				MAKE_ID('C','A','T',' ')
#define ID_NULL  			MAKE_ID(' ',' ',' ',' ')
#define ID_FORM_XMID_TIMB 	MAKE_ID('T','I','M','B') 
#define ID_FORM_XMID_RBRN 	MAKE_ID('R','B','R','N') 
#define ID_FORM_XMID_EVNT 	MAKE_ID('E','V','N','T')

typedef int8 IFF_ID[4];

typedef struct IFFCHUNK 
{
	IFF_ID id;
	int32 size; 	
	uint8 *data; 	
} sIffChunk;

sIffChunk *getXmidiTrackStart(const uint16 trackNo, sIffChunk *chunk);
int16 processXmidiTrackData(const uint16 trackNo, sIffChunk *firstChunk, sSequence_t **ppCurSequence);
int16 processXmidiTrackEvents(const uint16 trackNo, sIffChunk *trackDataStart, sSequence_t **ppCurSequence);

int16 processXmidiEvnt(sIffChunk *eventChunk, const uint16 trackNo, sSequence_t **ppCurSequence);
int16 processXmidiRbrn(sIffChunk *eventChunk, const uint16 trackNo, sSequence_t **ppCurSequence);
int16 processXmidiTimb(sIffChunk *eventChunk, const uint16 trackNo, sSequence_t **ppCurSequence);

static Bool isXmidiEventChunk(const uint32 id)
{
	Bool isXmidiEvent = false;
	
	if( id == ID_FORM_XMID_EVNT || id == ID_FORM_XMID_RBRN || id == ID_FORM_XMID_TIMB)
	{
		isXmidiEvent = true;
	}

	return isXmidiEvent;
}

static AM_INLINE int32 roundUp(const int32 val)
{
   if((val % 2)==0) return 0;
   return 1; // padded data
}

uint16 amGetNbOfXmidiTracks(void *midiData)
{
	uint16 nbOfXmidiForms = 0;

	sIffChunk *chunk = (sIffChunk *)midiData;
	const uint32 iffId = *((uint32 *)chunk->id);

	if ( iffId == ID_FORM)  
	{
  		if( (uint32)chunk->data == ID_FORM_XDIR)
  		{
			// get nb of XMID FORMS
			const sIffChunk *infoChunk = (sIffChunk *)(((uintptr)&chunk->data) + sizeof(uint32));

  			if(*((uint32 *)chunk->id) == ID_FORM_XDIR_INFO)
  			{
  				uint16 nbOfXmidiFiles = (uint16)(((uintptr)chunk->data>>16)&0x0000FFFF);
  				nbOfXmidiForms = ReadLE16(nbOfXmidiFiles);
  			}
  			else
  			{
				nbOfXmidiForms = 1;
  			}
   		} 
    } 
	else if(iffId == ID_CAT)
	{
  		if((uint32)chunk->data == ID_XMID)
  		{
  			// check if we have FORM<len>XMID 
  			chunk = (sIffChunk *)(((uintptr)&chunk->data) + sizeof(uint32));
    				
    		if( (uint32)chunk->data == ID_FORM) 
    		{
  				chunk = (sIffChunk *)(((uintptr)&chunk->data) + sizeof(uint32));

  				if( (uint32)chunk->data == ID_FORM) 
    			{
    				chunk = (sIffChunk *)(((uintptr)&chunk->data) + sizeof(uint32));
	
    				if( (uint32)chunk->data == ID_XMID) 
    				{
   						nbOfXmidiForms = 1;
   					}
				}
     		}
  		}
	}

	return nbOfXmidiForms;
 }

sIffChunk *getXmidiTrackStart(const uint16 trackNo, sIffChunk *chunk)
{
	sIffChunk *trackStart = 0;

	// TODO: iterate through FORM<len>XMID chunks and set start to trackNo, 
    // for now we handle only one sequence, first one we can find
  	chunk = (sIffChunk *)(((uintptr)chunk) + sizeof(uint32));
    const uint32 id = *((uint32 *)chunk->id);
    
    if( id == ID_FORM) 
    {
    	if((uint32)chunk->data == ID_XMID)
    	{
    		// FORM<len>XMID chunk found!
    		trackStart = chunk;
    	}
      	
	}

	return trackStart;
}

int16 processXmidiTrackData(const uint16 trackNo, sIffChunk *firstChunk, sSequence_t **ppCurSequence)
{
	const uint32 iffId = *((uint32 *)firstChunk->id);
	int16 retVal = AM_OK;

	if ( iffId == ID_FORM)  
	{
  		if( (uint32)firstChunk->data == ID_FORM_XDIR)
  		{
			// skip info chunk, check next chunk for xmidi cat after xdir
    		uint32 chunkSize = ReadBE32(firstChunk->size) + roundUp(firstChunk->size);
    		sIffChunk *chunk = (sIffChunk *)((uintptr)firstChunk + (uintptr)chunkSize + 8);

    		if((*((uint32 *)chunk->id) == ID_CAT) && ((uint32)chunk->data == ID_XMID))
    		{
   				chunk = getXmidiTrackStart(trackNo,(sIffChunk *)&chunk->data);

   				if(chunk)
   				{
   					retVal = processXmidiTrackEvents(trackNo, chunk, ppCurSequence);
   				}
   				else
   				{
   					amTrace("Error: XMidi track chunk not found!"NL,0);
   					retVal=AM_ERR;
   				}
    		}
   		}
   		else
   		{
   			amTrace("Error: XMidi XDIR chunk not found!"NL,0);
   			retVal=AM_ERR;
   		}
    } 
	else if(iffId == ID_CAT)
	{
  		if( (uint32)firstChunk->data == ID_XMID) 
      	{
     		sIffChunk *chunk = getXmidiTrackStart(trackNo,(sIffChunk *)&firstChunk->data);

     		if(chunk) 
     		{
     			retVal = processXmidiTrackEvents(trackNo, chunk, ppCurSequence);
     		}
     		else
   			{
   				amTrace("Error: XMidi track chunk not found!"NL,0);
   				retVal=AM_ERR;
   			}
      	}
	}

	return retVal;
}

// 
int16 processXmidiTrackEvents(const uint16 trackNo, sIffChunk *trackDataStart, sSequence_t **ppCurSequence)
{
	int16 ret = AM_OK;

	// trackData Start should be at FORM<len>XMID chunk..
	if( *((uint32 *)trackDataStart->id) == ID_FORM && ((uint32)trackDataStart->data) == ID_XMID )
	{
	    uint32 trackChunkSize = ReadBE32(trackDataStart->size) + roundUp(trackDataStart->size);
		sIffChunk *eventChunk = (sIffChunk *)(((uintptr)&trackDataStart->data) + sizeof(uint32)); // skipping XMID id
		
		// process event
		amTrace("Processing Xmidi events..."NL,0);

		uint32 id = *((uint32 *)eventChunk->id);
		uint32 eventChunkSize = ReadBE32(eventChunk->size) + roundUp(eventChunk->size);

		while(trackChunkSize>0 && (ret == AM_OK))
		{
	
			const Bool isXmidiEvent = isXmidiEventChunk(id);
			
			if(isXmidiEvent)
			{
				switch(id)
				{
					case ID_FORM_XMID_TIMB:
					{
						ret = processXmidiTimb(eventChunk, trackNo, ppCurSequence);
					} break;
					case ID_FORM_XMID_RBRN:
					{
						ret = processXmidiRbrn(eventChunk, trackNo, ppCurSequence);
					} break;
					case ID_FORM_XMID_EVNT:
					{
						ret = processXmidiEvnt(eventChunk, trackNo, ppCurSequence);
					} break;
				};
			}

			eventChunkSize = ReadBE32(eventChunk->size) + roundUp(eventChunk->size);
			const uint32 offset = eventChunkSize + 8;
			trackChunkSize -= offset;

			// go to next chunk, get id, event address + chunk size + size of id + size of uint32
			eventChunk = (sIffChunk *)(((uintptr)eventChunk) + offset);
			id = *((uint32 *)eventChunk->id);
		}
	}
	else
	{
		amTrace("Xmidi track data not found..."NL,0);
	}

    return ret;
}


Bool amIsValidXmidiData(void *midiData)
{
    Bool isValid = FALSE;

  	const sIffChunk *firstChunk = (sIffChunk *)midiData;
  	const uint32 iffId = *((uint32 *)firstChunk->id);

	if ( iffId == ID_FORM)  
	{
  		if( (uint32)firstChunk->data == ID_FORM_XDIR)
  		{
			// skip INFO chunk, check next chunk for xmidi cat after xdir
    		const uint32 chunkSize = ReadBE32(firstChunk->size) + roundUp(firstChunk->size);
    		sIffChunk* chunk = (sIffChunk *)((uintptr)firstChunk + (uintptr)chunkSize + 8);

    		if(*((uint32 *)chunk->id) == ID_CAT)
    		{
      			if( (uint32)chunk->data != ID_XMID) 
      				isValid = FALSE;
    		}

 			isValid = TRUE;
   		} 
  
	} 
	else if(iffId == ID_CAT)
	{
  		if((uint32)firstChunk->data == ID_XMID)
  		{
    		isValid = TRUE;
  		}
	}

 return isValid;
}

int16 amProcessXmidiData(void *data, const uint32 dataLength, sSequence_t **ppCurSequence)
{
	int32 chunkSize = 0;
	int16 ret=AM_OK;
	
	const void *endData = (void *)( (uintptr)data + (uintptr)dataLength );
	sIffChunk *fc = (sIffChunk *)data;
	const uint16 nbOfTracks=(*ppCurSequence)->ubNumTracks;

	(*ppCurSequence)->pSequenceName=0;
	(*ppCurSequence)->timeElapsedFrac=0;
	(*ppCurSequence)->timeStep=0;
	(*ppCurSequence)->timeDivision=0;
	(*ppCurSequence)->ubActiveTrack=0;
   	(*ppCurSequence)->seqType = nbOfTracks>1 ? ST_MULTI_SUB : ST_SINGLE;
 
	for(uint16 i=0;i < nbOfTracks; ++i){
		ret = processXmidiTrackData(i,fc,ppCurSequence);
		if(ret != AM_OK) break;
	}

	return ret;
}

/* [ TIMB<len> UWORD # of timbre list entries, 0-16384 
			{ 
				UBYTE patch number 0-127 
				UBYTE timbre bank 0-127 
			} 
			... 
] 
*/

int16 processXmidiTimb(sIffChunk *eventChunk, const uint16 trackNo, sSequence_t **ppCurSequence)
{
	const uint16 TimbreListEntriesNb = (uint16)(((uintptr)eventChunk->data>>16)&0x0000FFFF);
	const uint32 chunkSize = ReadBE32(eventChunk->size) + roundUp(eventChunk->size);
	
	amTrace("Timbre list entries size:[%d]: %u [0-16384]"NL, chunkSize, TimbreListEntriesNb);

	uint16 *timbreData = (uint16 *)((uintptr)&eventChunk->data + sizeof(uint32));

	for(uint16 idx=0; idx<TimbreListEntriesNb; ++idx)
	{
		uint8 patchNo = ((*(timbreData))>>8)&0x00FF;
		uint8 timbreBank = (*(timbreData))&0x00FF;

		amTrace("[%d] Timbre, patch no: %u timbre bank: %u"NL, idx, patchNo, timbreBank);

		++timbreData;
	}

	return AM_OK;
}

/* [ RBRN<len> UWORD # of branch point offsets, 0-127 
		{ 
			UWORD Sequence Branch Index controller value 0-127 
			ULONG controller offset from start of EVNT chunk 
		} 
		... 
	] 
*/

int16 processXmidiRbrn(sIffChunk *eventChunk, const uint16 trackNo, sSequence_t **ppCurSequence)
{
	const uint16 BranchPointOffsets = (uint16)(((uintptr)eventChunk->data>>16)&0x000000FF);
	const uint32 chunkSize = ReadBE32(eventChunk->size) + roundUp(eventChunk->size);
	amTrace("Branch point offsets size:[%d]: %u [0-127]"NL, chunkSize, BranchPointOffsets);
	return AM_OK;
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

/*	EVNT<len> 
	{ 
		UBYTE interval count (if < 128) 
		UBYTE <MIDI event> (if > 127)
	} 
*/

int16 processXmidiEvnt(sIffChunk *eventChunk, const uint16 trackNo, sSequence_t **ppCurSequence)
{
	const uint32 chunkSize = ReadBE32(eventChunk->size) + roundUp(eventChunk->size);
	
	amTrace("Events block size:[%d]:"NL, chunkSize);

	//todo: make it mc68000 friendly
	uint8 *eventData = (uint8 *)((uintptr)&eventChunk->data);
	uint32 offset=0;

	for(uint32 eventId=0; eventId < chunkSize; ++eventId)
	{
		uint8 val = *eventData;
		
		if(val<128)
		{
			amTrace("[EVNT] offset: [%u], int count: [%u]"NL, offset, val);
		}
		else
		{
			amTrace("[EVNT] offset: [%u], midi event: [%u]"NL, offset, val);
		}

		offset+=1;
		
		++eventData;
	}


	return AM_OK;
}
