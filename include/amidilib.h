
/**  Copyright 2007-2012 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#ifndef __AMIDILIB_H__
#define __AMIDILIB_H__

#include "c_vars.h"
#include "memory/memory.h"
#include "fmio.h"
#include "roland.h"
#include "midi.h"
#include "xmidi.h"
#include "midi_cmd.h"
#include "midi_rep.h"
#include "amidiseq.h"
#include "list/list.h"
#include "timing/miditim.h"

#define LIB_NAME "AMIDILIB : Atari (X)MIDI replay library\n"
#define AMIDI_INFO "(c)2007-2012 Pawel Goralski\ne-mail: nokturnal@nokturnal.pl\n"

#define AMIDI_MAJOR_VERSION 0
#define AMIDI_MINOR_VERSION 9
#define AMIDI_PATCHLEVEL 0

/* current version info struct */
typedef struct AMIDI_version {
	U16 major;
	U16 minor;
	U16 pad;
	U16 patch;
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

/* some messages are ignored by specified sound source so why bother to process them? */
/* second thing: sometimes we need different treatment for GS and LA sound sources */
typedef enum _midiDeviceTypes{
  DT_LA_SOUND_SOURCE=0,     /* native MT32 */
  DT_LA_SOUND_SOURCE_EXT,   /* for extended MT 32 modules with extra patches like CM-32L/CM-64 */
  DT_GS_SOUND_SOURCE,       /* for pure GS/GM sound source */
  DT_LA_GS_MIXED,           /* if both LA/GS sound sources are available, like in CM-500 */
  DT_MT32_GM_EMULATION,     /* before loading midi data MT32 sound banks has to be patched */
  DT_XG_GM_YAMAHA,
  DT_NUM_DEVICES	    /* before loading midi data MT32 sound banks has to be patched */
} eMidiDeviceType;

/** for internal use, midi file types  */
typedef enum _midiFileTypes{
  T_MIDI0=0,
  T_MIDI1,
  T_MIDI2,
  T_XMIDI,
  T_RMID,  /** RMID little endian M$hit invention */
  T_SMF,
  T_XMF,
  T_SNG,
  T_MUS,	/* e.g Doom MIDI music format */
  T_MIDIFTMAX  
} eMidiFileTypes;

/* function prototypes  */
/**
 * Returns AMIDI version info.
 * @return version info structure
 */

const sAMIDI_version *am_getVersionInfo(void);

/**
 * Gets MIDI file info type(0,1,2), number of tracks, track lenghts..
 *
 * @param pMidiPtr	gets info about the midi header
 * @return (0)- MIDI file type 0,
 * (1) - MIDI file type 1,
 * (2) - MIDI file type 2,
 * (3) - Miles Sound system XMIDI
 * (-1) - not MIDI file,
 * (-2) - not recognized MIDI file type format
 */

S16 am_getHeaderInfo(void *pMidiPtr);

/**
 * handles loaded into the memory (X)MIDI file
 *
 * @param pMidi	memory pointer where midi file is stored,
 * @param type - type of MIDI file to handle (0,1,2 supported), XMIDI.
 *        Value is returned by am_getHeaderInfo() function.
 * @param lenght - lenght of (X)MIDI block memory size in bytes
 * @param pSequence - pointer to AMIDI sequence structure to fill (needed to play our tune)
 * @return returns 0 if everything is OK, -1 if error occured
 **/

S16 am_handleMIDIfile(void *pMidiPtr, U32 lenght, sSequence_t **pSequence);

/**
 * gets number of tracks in MIDI file
 *
 * @param pMidi	memory pointer where (X)Midi file is stored.
 * @param type data type: MIDI 0,1,2 or XMIDI. Data type is returned by am_getHeaderInfo() function.
 * @return number of tracks in loaded MIDI file, -1 if error occured
 */

S16 am_getNbOfTracks(void *pMidiPtr, S16 type);

/**
 * gets time division from MIDI header data
 *
 * @param pMidi	memory pointer where (X)Midi file is stored.
 * @return number of tracks in loaded MIDI file, -1 if error occured
 */

U16 am_getTimeDivision (void *pMidiPtr);

/**
* Reads Variable lenght value used in MIDI files. Returns value in Variable lenght format
* @param none
**/
extern U32 am_readVarLen();


/** Calculates checksum of Roland SysEx messages
*	@param buf_start pointer to the start of SysEx message buffer
*	@param buf_end pointer to an end of SysEx message buffer
*	@return calculated checksum */
U8 am_calcRolandChecksum(U8 *buf_start, U8 *buf_end);

/** Inits system, set ups new, larger 32k MIDI system buffer
*	@return 1 if everything went ok */
S16 am_init(void);

/** dumps buffer to screen  */
void am_dumpMidiBuffer();

/** Deinits system, restores standard MIDI buffer
*/
void am_deinit(void);

/** processes the MIDI track data
*	@param startPtr pointer to the start of Track MIDI chunk
*	@param fileTypeFlag kind of file to preprocess 
*	@param numTracks number of track in given file to preprocess
*	@param pCurSequence pointer to AMIDI sequence structure  
*	@return pointer to the next chunk or NULL if EOT occured.
*/
void *processMidiTrackData(void *startPtr, U32 fileTypeFlag,U32 numTracks, sSequence_t **pCurSequence);

void *processMIDItrackEvents(void**startPtr, const void *endAddr, sTrack_t **pCurTrack);

/** read MIDI Variable lenght quantity
*  @param pChar pointer to VLQ data
*  @param ubSize size of VLQ data
*  @return decoded VLQ value
*/

U32 readVLQ(U8 *pChar,U8 *ubSize);

/** read/decode note off message
*   @param pPtr pointer to the pointer with midi data to read
*   @param recallRS current recall running status
*   @param delta current event delta
*   @param pCurTrack pointer to track structure where event will be stored
*/
void am_noteOff(U8 **pPtr,U16 *recallRS, U32 delta, sTrack_t **pCurTrack);

/** read/decode note on message
*   @param pPtr pointer to the pointer with midi data to read
*   @param recallRS current recall running status
*   @param delta current event delta
*   @param pCurTrack pointer to track structure where event will be stored
*/
void am_noteOn(U8 **pPtr,U16 *recallRS, U32 delta, sTrack_t **pCurTrack);

/** read/decode note aftertouch/pressure message
*   @param pPtr pointer to the pointer with midi data to read
*   @param recallRS current recall running status
*   @param delta current event delta
*   @param pCurTrack pointer to track structure where event will be stored
*/
void am_noteAft(U8 **pPtr,U16 *recallRS, U32 delta, sTrack_t **pCurTrack);

/** read/decode control change message
*   @param pPtr pointer to the pointer with midi data to read
*   @param recallRS current recall running status
*   @param delta current event delta
*   @param pCurTrack pointer to track structure where event will be stored
*/
void am_Controller(U8 **pPtr,U16 *recallRS, U32 delta, sTrack_t **pCurTrack);

/** read/decode program change message
*   @param pPtr pointer to the pointer with midi data to read
*   @param recallRS current recall running status
*   @param delta current event delta
*   @param pCurTrack pointer to track structure where event will be stored
*/
void am_PC(U8 **pPtr,U16 *recallRS, U32 delta, sTrack_t **pCurTrack);

/** read/decode channel aftertouch/pressure message
*	@param pPtr pointer to the pointer with midi data to read
*   @param recallRS current recall running status
*   @param delta current event delta
*   @param pCurTrack pointer to track structure where event will be stored
*/
void am_ChannelAft(U8 **pPtr,U16 *recallRS, U32 delta, sTrack_t **pCurTrack);

/** read/decode pitch bend message
*   @param pPtr pointer to the pointer with midi data to read
*   @param recallRS current recall running status
*   @param delta current event delta
*   @param pCurTrack pointer to track structure where event will be stored
*/
void am_PitchBend(U8 **pPtr,U16 *recallRS, U32 delta, sTrack_t **pCurTrack);

/** read/decode sysex message
*   @param pPtr pointer to the pointer with midi data to read
*   @param recallRS current recall running status
*   @param delta current event delta
*   @param pCurTrack pointer to track structure where event will be stored
*/
void am_Sysex(U8 **pPtr, U32 delta, sTrack_t **pCurTrack);

/** read/decode meta event message
*   @param pPtr pointer to the pointer with midi data to read
*   @param recallRS current recall running status
*   @param delta current event delta
*   @param pCurTrack pointer to track structure where event will be stored
*   @return bool flag TRUE if end of track (EOT) occured, FALSE otherwise.
*/
BOOL am_Meta(U8 **pPtr, U32 delta, sTrack_t **pCurTrack);

/** checks if byte is midi channel
*	@param byteEvent command byte
*	@return 1 if true, 0 otherwise
*/
U8 am_isMidiChannelEvent(U8 byteEvent);

/** checks if byte is Sysex or System realtime command byte
*	@param byteEvent pointer to VLQ data
*	@return 1 if true, 0 otherwise
*/
U8 am_isMidiRTorSysex(U8 byteEvent);

/* combine bytes function (14 bit values) for pitch bend */
U16 combineBytes(U8 bFirst, U8 bSecond);

/* gets MIDI controller name */
const U8 *getMIDIcontrollerName(U8 iNb);

/* returns info about connected devices  */
const S8 *getConnectedDeviceInfo(void);

/** Silents midi channels (n to n-1) - useful when we have for example hanging notes.
*   @param numChannels - number of channel
*/
void am_allNotesOff(U16 numChannels);

/** returns null terminated string with note name according to ASA ISO 
*   @param ubNoteNb - note number in 0-127 range
*/
const U8 *am_getMIDInoteName(U8 ubNoteNb);

void getDeviceInfoResponse(U8 channel);

const char *getNoteName(U8 currentChannel,U8 currentPN, U8 noteNumber);

/** destroys loaded Sequence. 
*   @param pPtr pointer to a pointer with loaded sequence data. Passed pointer is null
*   if operation was completed succesfully. */

void am_destroySequence(sSequence_t **pPtr);

/** returns meaningful name for Midi Device type enumeration.  */
const U8 *am_getMidiDeviceTypeName(eMidiDeviceType device);

#ifdef DEBUG_BUILD
/** function for variable quantity reading test */
void VLQtest(void);
#endif


#endif

