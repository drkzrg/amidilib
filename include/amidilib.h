/**  Copyright 2007-2021 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#ifndef _AMIDILIB_H_
#define _AMIDILIB_H_

#include "vartypes.h"
#include "midi_rep.h"
#include "mdevtype.h"

#define LIB_NAME "AMIDILIB : (X)MIDI replay library\n"
#define AMIDI_INFO "(c)2007-2021 Pawel Goralski\n"

#define AMIDI_MAJOR_VERSION 1
#define AMIDI_MINOR_VERSION 4
#define AMIDI_PATCHLEVEL 0

/* current version info struct */
typedef struct AMIDI_version {
	uint16 major;
	uint16 minor;
	uint16 pad;
	uint16 patch;
} sAMIDI_version;

#define AMIDI_VERSION(X) \
{\
(X)->major = AMIDI_MAJOR_VERSION; \
(X)->minor = AMIDI_MINOR_VERSION; \
(X)->patch = AMIDI_PATCHLEVEL; }

/* This macro turns the version numbers into a numeric value: (1,2,3) -> (1203)
   This assumes that there will never be more than 100 patchlevels
*/
#define AMIDI_VERSIONNUM(X, Y, Z)						\
	((X)*1000 + (Y)*100 + (Z))

/* This is the version number macro for the current AMIDI version */
#define AMIDI_COMPILEDVERSION \
	AMIDI_VERSIONNUM(AMIDI_MAJOR_VERSION, AMIDI_MINOR_VERSION, AMIDI_PATCHLEVEL)

/* This macro will evaluate to true if compiled with AMIDI at least X.Y.Z */
#define AMIDI_VERSION_ATLEAST(X, Y, Z) \
	(AMIDI_COMPILEDVERSION >= AMIDI_VERSIONNUM(X, Y, Z))

/* function prototypes  */
/**
 * Returns AMIDI version info.
 * @return version info structure
 */
const sAMIDI_version *amGetVersionInfo(void);

/**
 * processes loaded midi file data to amidilib sequence
 *
 * @param pFilename	- midi file name
 * @param midiData - pointer to loaded midi file data
 * @param midiDataSize - length of midi data in bytes
 * @param ppSequence - pointer to a an adress containing AMIDI sequence to populate
 * @return returns AM_OK if everything is OK, AM_ERR if error occured
 **/
retVal amProcessMidiFileData(const char *filename, void *midiData, const uint32 midiDataSize, sSequence_t **ppSequence);

/** Inits system, set ups new, larger 32k MIDI system buffer
*	@return 1 if everything went ok */
retVal amInit(void);

/** Deinits system, restores standard MIDI buffer */
void amDeinit(void);

/** returns info about connected devices  */
const int8 *amGetConnectedDeviceInfo(void);

void amGetDeviceInfoResponse(const uint8 channel);

/** returns meaningful name for Midi Device type enumeration.  */
const uint8 *amGetMidiDeviceTypeName(const eMidiDeviceType device);

#endif
