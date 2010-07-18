
/**  Copyright 2007-2010 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#ifndef __AMIDILIB_H__
#define __AMIDILIB_H__

#include "c_vars.h"
#include "midi.h"
#include "xmidi.h"
#include "fmio.h"
#include "roland.h"
#include "amidiseq.h"
#include "midi_cmd.h"
#include <time.h>

#define LIB_NAME "Atari (X)MIDI library"
#define AMIDI_INFO "(c)2008-2009 Pawel Goralski e-mail: nokturnal@nokturnal.pl"

#define AMIDI_MAJOR_VERSION 1
#define AMIDI_MINOR_VERSION 2
#define AMIDI_PATCHLEVEL 0

#define AMIDI_VERSION(X)/{\
(X)->major = AMIDI_MAJOR_VERSION;\
(X)->minor = AMIDI_MINOR_VERSION;\
(X)->patch = AMIDI_PATCHLEVEL;\
}

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

#define MIDI_FLOWCTRL 0		/* flow control enable/disable, default by: disabled*/
#define MIDI_LWM 32		/* low watermark if flow control enabled */
#define MIDI_HWM 32000		/* hight watermark if flow control enabled */
#define MIDI_BUFFER_SIZE 32767  /*default MIDI buffer size 32k */


/* some messages are ignored by specified sound source so why bother to process them? */
/* second thing: sometimes we need different treatment for GS and LA sound sources */

#define LA_SOUND_SOURCE 0       /* native MT32 */
#define LA_SOUND_SOURCE_EXT 1   /* for extended MT 32 modules with extra patches like CM-32L/CM-64 */
#define GS_SOUND_SOURCE 2       /* for pure GS/GM sound source */
#define LA_GS_MIXED 4           /* if both LA/GS sound sources are available, like in CM-500 */
#define MT32_GM_EMULATION 5     /* before loading midi data MT32 sound banks has to be patched */
#define XG_GM_YAMAHA 6		/* before loading midi data MT32 sound banks has to be patched */


/** for internal use, midi file types  */
#define T_MIDI0   0
#define T_MIDI1   1
#define T_MIDI2   2
#define T_XMIDI   3
#define T_RMID    4  /** RMID little endian M$hit invention */
#define T_SMF     5
#define T_XMF     6
#define T_SNG     7
#define T_MUS     8	/* e.g Doom MIDI music format */

/* MIDI macros */
/* returns != 0 if data are in system MIDI buffer */
#define MIDI_DATA_READY Bconstat(DEV_MIDI)
/* sends 1 byte to MIDI output */
#define MIDI_SEND_BYTE(data) Bconout(DEV_MIDI,(U8)data)
/* sends multiple bytes to MIDI output, count is number of bytes to send */
#define MIDI_SEND_DATA(count,data) Midiws(count-1,data)
/* reads 1 unsigned byte from MIDI input */
#define GET_MIDI_DATA        (U8)Bconin(DEV_MIDI)


/* small trick to get rid of logs/debug info in final build :D haxx0r! let the compiler do dirty work */
#if DEBUG_BUILD
#define amTrace am_log
#else
#define amTrace sizeof
#endif



/* function prototypes  */
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

S16 am_handleMIDIfile(void *pMidiPtr, S16 type, U32 lenght, sSequence_t *pSequence);

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
 * Prints out MIDI file track info
 *
 * @param pMidiPtr memory pointer where midi file is stored.
 * @param usiTrackNb which Track info to fetch out (1 - 65,535)
 * @return
 */
 S16 am_getTrackInfo(void *pMidiPtr, U16 usiTrackNb, sMIDItrackInfo *pTrackInfo);

/**
 * Returns pointer to specified MIDI track
 * @param iTrackNum	MIDI track number
 * @param pMidiPtr	memory pointer where midi file is stored.
 */

void *am_getTrackPtr(void *pMidiPtr,S16 iTrackNum);

/**
* Reads Variable lenght value used in MIDI files. Returns value in Variable lenght format
* @param none
**/
extern U32 am_readVarLen();

/**
 * Sends request to identify all attached MIDI devices.
 * Returns pointer to linked list with all MIDI devices attached, NULL if no devices are attached
**/
extern void *am_checkDevicesIdentity (void);

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
void *processMidiTrackData(void *startPtr, U32 fileTypeFlag,U32 numTracks, sSequence_t *pCurSequence);

void *processMIDItrackEvents(void**startPtr, const void **endAddr, sTrack_t **pCurTrack);

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

/** Calculate timestep function
*   BPM - beats per minute (tempo)
*   PPU=QPM * PPQ *(1/UPS)*(1 minute /60 seconds)
*   @param ups UPS - update interval
*   @param PPQ - parts per quaternote
*   @param qpm QPM - quaternotes per minute
*/
U32 am_calculateTimeStep(U16 qpm, U16 ppq, U16 ups);

/** Calculate timestep function (float version)
*   BPM - beats per minute (tempo)
*   PPU=QPM * PPQ *(1/UPS)*(1 minute /60 seconds)
*   @param ups UPS - update interval
*   @param PPQ - parts per quaternote
*   @param qpm QPM - quaternotes per minute
*/
float am_calculateTimeStepFlt(U16 qpm, U16 ppq, U16 ups);

/** Simple time division to console function
*   with example of handling timeDivision data
*   @param timeDivision - time division value from midi file header
*   @return decoded time division in PPQN or FPS
*/

U16 am_decodeTimeDivisionInfo(U16 timeDivision);

/** Silents midi channels (n to n-1) - useful when we have for example hanging notes.
*   @param numChannels - number of channel
*/
void am_allNotesOff(U16 numChannels);

/** utility for measuring function execution time (end-begin timestamp in ms) 
*   @param end - end timestamp
*   @param begin - begin timestamp
*/
double am_diffclock(clock_t end,clock_t begin);

/** returns null terminated string with note name according to ASA ISO 
*   @param ubNoteNb - note number in 0-127 range
*/
const U8 *am_getMIDInoteName(U8 ubNoteNb);

/** Utility function sends text buffer to console and text log.
*   @param mes - null terminated string
*/
void am_log(const U8 *mes,...);


void hMidiEvent(void);	/* handles stuff inside the replay routine */ 
void getDeviceInfoResponse(U8 channel);
void playSequence(const sEventList **listPtr);

const char *getNoteName(U8 currentChannel,U8 currentPN, U8 noteNumber);

#endif
