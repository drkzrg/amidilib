
/**  Copyright 2007-2020 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#ifndef __AMIDI_H__
#define __AMIDI_H__

#include "c_vars.h"
#include "vendors.h"
#include "events.h"

#define ID_MTHD     0x4D546864UL
#define ID_MTRK     0x4D54726BUL
#define ID_MTRK_END 0x0054726BUL

#define MIDI_MAXCHANNELS	16

/**
 * Time Division
*/

typedef enum{
 TD_TICKS_PER_BEAT=0,
 TD_SMPTE=1
} eTimeDivision;

/**
 * MIDI file header struct
* these structures need to be packed 
*/

typedef struct PACK_ATTR ChunkHeader {
  uint32 id;
  uint32 headLenght;
} sChunkHeader,*pChunkHeader;

StaticAssert((sizeof(sChunkHeader)==8),"Invalid sChunkHeader size.");


typedef struct PACK_ATTR MThd {
	uint32 id;
	uint32 headLenght;
	uint16 format;
	uint16 nTracks;
	uint16 division;
} sMThd;

StaticAssert(sizeof(sMThd)==14,"Invalid sMThd size.");


/**
 * MIDI file track info struct
 */

typedef struct PACK_ATTR MTrk {
  uint32 id;
	uint32 headLenght;
	/* offset track event data 0x08 offset */
} sMTrk;

StaticAssert(sizeof(sMTrk)==8,"Invalid sMTrk size.");


/**
 * MIDI device info struct
*  custom structure for storing connected device info
*/

typedef struct PACK_ATTR DeviceInfo{
  uint8 nChannel;                  /* channel number on which device receives data */
  uint8 pad[4];
  uint16 mID;                      /* vendor id, complete list in VENDORS.H */
  uint16 deviceFamilyID;           /* ff ff	Device family code (14 bits, LSB first) */
  uint16 deviceFamilyMC;           /* dd dd	Device family member code (14 bits, LSB first) */
  uint16 SoftRevLevel;             /* ss ss ss ss	Software revision level (the format is device specific) */
} sDeviceInfo,*pDeviceInfo;

StaticAssert(sizeof(sDeviceInfo)==13,"Invalid sDeviceInfo size.");


/* SMPTE OFFSET struct */
typedef struct PACK_ATTR SMPTEoffset{
    uint8 hr;
    uint8 mn;
    uint8 se;
    uint8 fr;
    uint8 ff;
  } sSMPTEoffset;
StaticAssert(sizeof(sSMPTEoffset)==5,"Invalid sSMPTEoffset size.");

/* Time signature struct */

typedef struct PACK_ATTR TimeSignature{
 uint8 nn;
 uint8 dd;
 uint8 cc;
 uint8 bb;
} sTimeSignature;

StaticAssert(sizeof(sTimeSignature)==4,"Invalid sTimeSignature size.");

/*************** event structs */
typedef struct PACK_ATTR NoteOn_t{
 int8 noteNb;
 int8 velocity;
}  sNoteOn_t;

StaticAssert(sizeof(sNoteOn_t)==2,"Invalid sNoteOn_t size.");

typedef struct PACK_ATTR NoteOff_t{
 int8 noteNb;
 int8 velocity;
} sNoteOff_t;

StaticAssert(sizeof(sNoteOff_t)==2,"Invalid sNoteOff_t size.");


typedef struct PACK_ATTR NoteAft_t{
 int8 noteNb;
 int8 pressure;
} sNoteAft_t;

StaticAssert(sizeof(sNoteAft_t)==2,"Invalid sNoteAft_t size.");


typedef struct PACK_ATTR Controller_t{
 int8 controllerNb;
 int8 value;
} sController_t;

StaticAssert(sizeof(sController_t)==2,"Invalid sController_t size.");


typedef struct PACK_ATTR ProgramChange_t {
 int8 programNb;
} sProgramChange_t;

StaticAssert(sizeof(sProgramChange_t)==1,"Invalid sProgramChange_t size.");


typedef struct PACK_ATTR ChannelAft_t{
 int8 pressure;
} sChannelAft_t;

StaticAssert(sizeof(sChannelAft_t)==1,"Invalid sChannelAft_t size.");

typedef struct PACK_ATTR PitchBend_t{
 int8  LSB;
 int8  MSB;
} sPitchBend_t;

StaticAssert(sizeof(sPitchBend_t)==2,"Invalid sPitchBend_t size.");

typedef struct PACK_ATTR Tempo_t{
 uint32 tempoVal;
} sTempo_t;

StaticAssert(sizeof(sTempo_t)==4,"Invalid sTempo_t size.");

const uint8 *getMidiNoteName(const uint8 NoteNb);
const uint8 *getMidiControllerName(const uint8 NoteNb);

//returns note name or rhytm part name if current channel is 9
const uint8 *getNoteName(const uint8 currentChannel,const uint8 currentPN,const uint8 noteNumber);

/** read MIDI Variable lenght quantity
*  @param pVLQdata pointer to VLQ data
*  @param ubSize size of VLQ data
*  @return decoded VLQ value
*/

/* reads Variable Lenght Quantity */
static INLINE uint32 readVLQ(uint8 *pChar,uint8 *ubSize)
{
// TODO: rewrite this in assembly
uint32 value=0;
(*ubSize)=0;
value = (*pChar);

if ( (value & 0x80) ){
       value &= 0x7F;

/* get next byte */
pChar++;
(*ubSize)++;
  uint8 c=0;
       do{
	     value = (value << 7);
         c = (*pChar);
         value = value + (c&0x7F);
          pChar++;
          (*ubSize)++;
       } while (c & 0x80);
    }else{
     (*ubSize)++;
    }

return(value);
}

// reads a variable length integer
// TODO: remove it and replace with uint32 readVLQ(uint8 *pChar,uint8 *ubSize)
static INLINE uint32 ReadVarLen(int8* buffer)
{
uint32 value;

if ((value = *buffer++) & 0x80) {
  value &= 0x7f;
  uint8 c;
  do  {
	  
    value = (value << 7) + ((c = *buffer++) & 0x7f);
  } while (c & 0x80);
 }
 return value;
}

// Writes a variable length integer to a buffer, and returns bytes written
static INLINE int32 WriteVarLen( int32 value, uint8* out )
{
    int32 buffer, count = 0;

    buffer = value & 0x7f;

    while ((value >>= 7) > 0) {
        buffer <<= 8;
        buffer += 0x80;
        buffer += (value & 0x7f);
    }

    while (1) {
        ++count;
        *out = (uint8)buffer;
        ++out;
        if (buffer & 0x80)
            buffer >>= 8;
        else
            break;
 }
 return count;
}


#endif
