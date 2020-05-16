/**  Copyright 2007-2019 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#ifndef _AMIDILIB_H_
#define _AMIDILIB_H_

#include "c_vars.h"
#include "midi_rep.h"
#include "mdevtype.h"

#define LIB_NAME "AMIDILIB : (X)MIDI replay library\n"
#define AMIDI_INFO "(c)2007-2020 Pawel Goralski\n"

#define AMIDI_MAJOR_VERSION 1
#define AMIDI_MINOR_VERSION 3
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
 * handles loaded into the memory (X)MIDI file
 *
 * @param pMidi	memory pointer where midi file is stored,
 * @param type - type of MIDI file to handle (0,1,2 supported), XMIDI.
 *        Value is returned by amGetHeaderInfo() function.
 * @param lenght - lenght of (X)MIDI block memory size in bytes
 * @param pSequence - pointer to AMIDI sequence structure to fill (needed to play our tune)
 * @return returns 0 if everything is OK, -1 if error occured
 **/

int16 amLoadMidiFile(const char *pFilename, void *pMidiPtr, sSequence_t **pSequence);

/**
 * gets number of tracks in MIDI file
 *
 * @param pMidi	memory pointer where (X)Midi file is stored.
 * @param type data type: MIDI 0,1,2 or XMIDI. Data type is returned by amGetHeaderInfo() function.
 * @return number of tracks in loaded MIDI file, -1 if error occured
 */

/** Inits system, set ups new, larger 32k MIDI system buffer
*	@return 1 if everything went ok */
int16 amInit(void);

/** Deinits system, restores standard MIDI buffer
*/

void amDeinit(void);

/** returns info about connected devices  */
const int8 *amGetConnectedDeviceInfo(void);

void amGetDeviceInfoResponse(const uint8 channel);

/** returns meaningful name for Midi Device type enumeration.  */
const uint8 *amGetMidiDeviceTypeName(const eMidiDeviceType device);


#ifdef DEBUG_BUILD
/** function for variable quantity reading test */
void VLQtest(void);
#endif


#endif
