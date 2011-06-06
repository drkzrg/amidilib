
/**  Copyright 2007-2010 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
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

/**
 * Time Division
*/

typedef enum
{
 TD_TICKS_PER_BEAT=0,
 TD_SMPTE=1
} eTimeDivision;

/**
 * MIDI file header struct
 */

typedef struct ChunkHeader
{
  U32 id;
  U32 headLenght;
} __attribute__((packed)) sChunkHeader,*pChunkHeader;


typedef struct __attribute__((packed)) MThd{
	U32 id ;
	U32 headLenght;
	U16 format;
	U16 nTracks;
	U16 division;
} sMThd;


/**
 * MIDI file track info struct
 */

typedef struct __attribute__((packed)) MTrk
{	U32 id;
	U32 headLenght;
	/* offset track event data 0x08 offset */
} sMTrk;


/**
 * MIDI device info struct
*/

typedef struct __attribute__((packed)) DeviceInfo
{
  U8 nChannel;                  /* channel number on which device receives data */
  U8 pad[1];
  U16 mID;                      /* vendor id, complete list in VENDORS.H */
  U16 deviceFamilyID;           /* ff ff	Device family code (14 bits, LSB first) */
  U16 deviceFamilyMC;           /* dd dd	Device family member code (14 bits, LSB first) */
  U16 SoftRevLevel;             /* ss ss ss ss	Software revision level (the format is device specific) */
} sDeviceInfo,*pDeviceInfo;


/*
 * MIDI track info struct
*/
typedef struct __attribute__((packed)) MIDItrackInfo {
    U16 usiTrackNb;
    U16  pad;
    U32   ulTrackLenght;
    U32   trackOffset;      /* offset relative to start address of loaded midi file */
} sMIDItrackInfo,*pMidiTrackInfo;


/* SMPTE OFFSET struct */
typedef struct __attribute__((packed)) SMPTEoffset
 {
    U8 hr;
    U8 mn;
    U8 se;
    U8 fr;
    U8 ff;
    U8 pad[3];
} sSMPTEoffset;

/* Time signature struct */

typedef struct __attribute__((packed)) TimeSignature
{
 U8 nn;
 U8 dd;
 U8 cc;
 U8 bb;
} sTimeSignature;

/*************** event structs */
typedef struct 
{
 S8 noteNb;
 S8 velocity;
} __attribute__((packed)) sNoteOn_t;

typedef struct 
{
 S8 noteNb;
 S8 velocity;
}__attribute__((packed)) sNoteOff_t;

typedef struct 
{
 S8 noteNb;
 S8 pressure;
}__attribute__((packed)) sNoteAft_t;

typedef struct 
{
 S8 controllerNb;
 S8 value;
}__attribute__((packed)) sController_t;

typedef struct ProgramChange_t 
{
 S8 programNb;
}__attribute__((packed)) sProgramChange_t;

typedef struct 
{
 S8 pressure;
}__attribute__((packed)) sChannelAft_t;

typedef struct 
{
 S8  LSB;
 S8  MSB;
} __attribute__((packed)) sPitchBend_t;

typedef struct 
{
 U32 tempoVal;
} __attribute__((packed)) sTempo_t;

#endif
