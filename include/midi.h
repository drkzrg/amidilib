
/**  Copyright 2007-2011 Pawel Goralski
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

typedef enum{
 TD_TICKS_PER_BEAT=0,
 TD_SMPTE=1
} eTimeDivision;

/**
 * MIDI file header struct
* these structures need to be packed 
*/

typedef struct __attribute__((packed)) ChunkHeader{
  U32 id;
  U32 headLenght;
} sChunkHeader,*pChunkHeader;


typedef struct __attribute__((packed)) MThd{
	U32 id;
	U32 headLenght;
	U16 format;
	U16 nTracks;
	U16 division;
} sMThd;


/**
 * MIDI file track info struct
 */

typedef struct __attribute__((packed)) MTrk {
    U32 id;
	U32 headLenght;
	/* offset track event data 0x08 offset */
} sMTrk;


/**
 * MIDI device info struct
*  custom structure for storing connected device info
*/

typedef struct __attribute__((packed)) DeviceInfo{
  U8 nChannel;                  /* channel number on which device receives data */
  U8 pad[4];
  U16 mID;                      /* vendor id, complete list in VENDORS.H */
  U16 deviceFamilyID;           /* ff ff	Device family code (14 bits, LSB first) */
  U16 deviceFamilyMC;           /* dd dd	Device family member code (14 bits, LSB first) */
  U16 SoftRevLevel;             /* ss ss ss ss	Software revision level (the format is device specific) */
} sDeviceInfo,*pDeviceInfo;


/* SMPTE OFFSET struct */
typedef struct __attribute__((packed)) SMPTEoffset{
    U8 hr;
    U8 mn;
    U8 se;
    U8 fr;
    U8 ff;
  } sSMPTEoffset;

/* Time signature struct */

typedef struct __attribute__((packed)) TimeSignature{
 U8 nn;
 U8 dd;
 U8 cc;
 U8 bb;
} sTimeSignature;

/*************** event structs */
typedef struct __attribute__((packed)) NoteOn_t{
 S8 noteNb;
 S8 velocity;
}  sNoteOn_t;

typedef struct __attribute__((packed)) NoteOff_t{
 S8 noteNb;
 S8 velocity;
} sNoteOff_t;

typedef struct __attribute__((packed)) NoteAft_t{
 S8 noteNb;
 S8 pressure;
} sNoteAft_t;

typedef struct __attribute__((packed)) Controller_t{
 S8 controllerNb;
 S8 value;
} sController_t;

typedef struct __attribute__((packed)) ProgramChange_t {
 S8 programNb;
} sProgramChange_t;

typedef struct __attribute__((packed)) ChannelAft_t{
 S8 pressure;
} sChannelAft_t;

typedef struct __attribute__((packed)) PitchBend_t{
 S8  LSB;
 S8  MSB;
} sPitchBend_t;

typedef struct __attribute__((packed)) Tempo_t{
 U32 tempoVal;
} sTempo_t;


const U8 *getMidiNoteName(const U8 NoteNb);
const U8 *getMidiControllerName(const U8 NoteNb);

//returns note name or rhytm part name if current channel is 9
const U8 *getNoteName(const U8 currentChannel,const U8 currentPN,const U8 noteNumber);

#endif
