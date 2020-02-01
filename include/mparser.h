#ifndef MPARSER_H
#define MPARSER_H

/** main midi parser
    parses midi data and transforms it to custom sequence
*/

/**  Copyright 2007-2019 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#include "c_vars.h"
#include "amidiseq.h"
#include "mformats.h"

int16 amGetNbOfTracks(void *pMidiPtr, eMidiFileType type);

/**
 * gets time division from MIDI header data
 *
 * @param pMidi	memory pointer where (X)Midi file is stored.
 * @return number of tracks in loaded MIDI file, -1 if error occured
 */

uint16 amGetTimeDivision (const void *pMidiPtr);

/**
* Reads Variable lenght value used in MIDI files. Returns value in Variable lenght format
* @param none
**/
extern uint32 amReadVarLen(void);

/**
 * Gets MIDI file info type(0,1,2), number of tracks, track lenghts..
 *
 * @param pMidiPtr	gets info about the midi header
 * @return (0)- MIDI file type 0,
 * (1) - MIDI file type 1,
 * (2) - MIDI file type 2,
 * (3) - Miles Sound system XMIDI
 * (-1) - unknown MIDI file,
 */

eMidiFileType amGetHeaderInfo(const void *pMidiPtr);

/** processes the MIDI track data
*	@param startPtr pointer to the start of Track MIDI chunk
*	@param fileTypeFlag kind of file to preprocess
*	@param numTracks number of track in given file to preprocess
*	@param pCurSequence pointer to AMIDI sequence structure
*	@return pointer to the next chunk or NULL if EOT occured.
*/
void *processMidiTrackData(void *startPtr, const eMidiFileType fileTypeFlag, const uint32 numTracks, sSequence_t **pCurSequence, int16 *iError);

/** processes the MIDI track events
*	@param startPtr TODO
*	@param endAddr TODO
*	@param pCurTrack TODO
*/
void *processMIDItrackEvents(sSequence_t *pSeq, void**startPtr, const void *endAddr, sTrack_t **pCurTrack,int16 *iError);

/** read/decode note off message
*   @param pPtr pointer to the pointer with midi data to read
*   @param recallRS current recall running status
*   @param delta current event delta
*   @param pCurTrack pointer to track structure where event will be stored
*/
int16 amNoteOff(sSequence_t *pSeq, uint8 **pPtr,uint16 *recallRS, const uint32 delta, sTrack_t **pCurTrack);

/** read/decode note on message
*   @param pPtr pointer to the pointer with midi data to read
*   @param recallRS current recall running status
*   @param delta current event delta
*   @param pCurTrack pointer to track structure where event will be stored
*/
int16 amNoteOn(sSequence_t *pSeq, uint8 **pPtr,uint16 *recallRS, const uint32 delta, sTrack_t **pCurTrack);

/** read/decode note aftertouch/pressure message
*   @param pPtr pointer to the pointer with midi data to read
*   @param recallRS current recall running status
*   @param delta current event delta
*   @param pCurTrack pointer to track structure where event will be stored
*/
int16 amNoteAft(sSequence_t *pSeq, uint8 **pPtr,uint16 *recallRS, const uint32 delta, sTrack_t **pCurTrack);

/** read/decode control change message
*   @param pPtr pointer to the pointer with midi data to read
*   @param recallRS current recall running status
*   @param delta current event delta
*   @param pCurTrack pointer to track structure where event will be stored
*/
int16 amController(sSequence_t *pSeq, uint8 **pPtr,uint16 *recallRS, const uint32 delta, sTrack_t **pCurTrack);

/** read/decode program change message
*   @param pPtr pointer to the pointer with midi data to read
*   @param recallRS current recall running status
*   @param delta current event delta
*   @param pCurTrack pointer to track structure where event will be stored
*/
int16 amProgramChange(sSequence_t *pSeq, uint8 **pPtr,uint16 *recallRS, const uint32 delta, sTrack_t **pCurTrack);

/** read/decode channel aftertouch/pressure message
*	@param pPtr pointer to the pointer with midi data to read
*   @param recallRS current recall running status
*   @param delta current event delta
*   @param pCurTrack pointer to track structure where event will be stored
*/
int16 amChannelAft(sSequence_t *pSeq, uint8 **pPtr,uint16 *recallRS, const uint32 delta, sTrack_t **pCurTrack);

/** read/decode pitch bend message
*   @param pPtr pointer to the pointer with midi data to read
*   @param recallRS current recall running status
*   @param delta current event delta
*   @param pCurTrack pointer to track structure where event will be stored
*/
int16 amPitchBend(sSequence_t *pSeq, uint8 **pPtr,uint16 *recallRS, const uint32 delta, sTrack_t **pCurTrack);

/** read/decode sysex message
*   @param pPtr pointer to the pointer with midi data to read
*   @param recallRS current recall running status
*   @param delta current event delta
*   @param pCurTrack pointer to track structure where event will be stored
*/
int16 amSysexMsg(sSequence_t *pSeq, uint8 **pPtr, const uint32 delta, sTrack_t **pCurTrack);

/** read/decode meta event message
*   @param pPtr pointer to the pointer with midi data to read
*   @param recallRS current recall running status
*   @param delta current event delta
*   @param pCurTrack pointer to track structure where event will be stored
*   @return bool flag TRUE if end of track (EOT) occured, FALSE otherwise.
*/
int16 amMetaEvent(sSequence_t *pSeq, uint8 **pPtr, const uint32 delta, sTrack_t **pCurTrack, bool *bEOT);

/** checks if byte is midi channel
*	@param byteEvent command byte
*	@return 1 if true, 0 otherwise
*/
uint8 amIsMidiChannelEvent(const uint8 byteEvent);

/** checks if byte is Sysex or System realtime command byte
*	@param byteEvent pointer to VLQ data
*	@return 1 if true, 0 otherwise
*/
uint8 amIsMidiRTorSysex(const uint8 byteEvent);

/* combine bytes function (14 bit values) for pitch bend */
uint16 amCombinePitchBendBytes(const uint8 bFirst, const uint8 bSecond);

#endif // MPARSER_H
