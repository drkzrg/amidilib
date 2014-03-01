#ifndef MPARSER_H
#define MPARSER_H

/** main midi parser
    parses midi data and transforms it to custom sequence
*/

/**  Copyright 2007-2014 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#include "c_vars.h"
#include "amidiseq.h"

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

/** processes the MIDI track data
*	@param startPtr pointer to the start of Track MIDI chunk
*	@param fileTypeFlag kind of file to preprocess
*	@param numTracks number of track in given file to preprocess
*	@param pCurSequence pointer to AMIDI sequence structure
*	@return pointer to the next chunk or NULL if EOT occured.
*/
void *processMidiTrackData(void *startPtr, U32 fileTypeFlag,U32 numTracks, sSequence_t **pCurSequence, S16 *iError);

/** processes the MIDI track events
*	@param startPtr TODO
*	@param endAddr TODO
*	@param pCurTrack TODO
*/
void *processMIDItrackEvents(void**startPtr, const void *endAddr, sTrack_t **pCurTrack,S16 *iError);

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
S16 am_noteOff(U8 **pPtr,U16 *recallRS, U32 delta, sTrack_t **pCurTrack);

/** read/decode note on message
*   @param pPtr pointer to the pointer with midi data to read
*   @param recallRS current recall running status
*   @param delta current event delta
*   @param pCurTrack pointer to track structure where event will be stored
*/
S16 am_noteOn(U8 **pPtr,U16 *recallRS, U32 delta, sTrack_t **pCurTrack);

/** read/decode note aftertouch/pressure message
*   @param pPtr pointer to the pointer with midi data to read
*   @param recallRS current recall running status
*   @param delta current event delta
*   @param pCurTrack pointer to track structure where event will be stored
*/
S16 am_noteAft(U8 **pPtr,U16 *recallRS, U32 delta, sTrack_t **pCurTrack);

/** read/decode control change message
*   @param pPtr pointer to the pointer with midi data to read
*   @param recallRS current recall running status
*   @param delta current event delta
*   @param pCurTrack pointer to track structure where event will be stored
*/
S16 am_Controller(U8 **pPtr,U16 *recallRS, U32 delta, sTrack_t **pCurTrack);

/** read/decode program change message
*   @param pPtr pointer to the pointer with midi data to read
*   @param recallRS current recall running status
*   @param delta current event delta
*   @param pCurTrack pointer to track structure where event will be stored
*/
S16 am_PC(U8 **pPtr,U16 *recallRS, U32 delta, sTrack_t **pCurTrack);

/** read/decode channel aftertouch/pressure message
*	@param pPtr pointer to the pointer with midi data to read
*   @param recallRS current recall running status
*   @param delta current event delta
*   @param pCurTrack pointer to track structure where event will be stored
*/
S16 am_ChannelAft(U8 **pPtr,U16 *recallRS, U32 delta, sTrack_t **pCurTrack);

/** read/decode pitch bend message
*   @param pPtr pointer to the pointer with midi data to read
*   @param recallRS current recall running status
*   @param delta current event delta
*   @param pCurTrack pointer to track structure where event will be stored
*/
S16 am_PitchBend(U8 **pPtr,U16 *recallRS, U32 delta, sTrack_t **pCurTrack);

/** read/decode sysex message
*   @param pPtr pointer to the pointer with midi data to read
*   @param recallRS current recall running status
*   @param delta current event delta
*   @param pCurTrack pointer to track structure where event will be stored
*/
S16 am_Sysex(U8 **pPtr, U32 delta, sTrack_t **pCurTrack);

/** read/decode meta event message
*   @param pPtr pointer to the pointer with midi data to read
*   @param recallRS current recall running status
*   @param delta current event delta
*   @param pCurTrack pointer to track structure where event will be stored
*   @return bool flag TRUE if end of track (EOT) occured, FALSE otherwise.
*/
S16 am_Meta(U8 **pPtr, U32 delta, sTrack_t **pCurTrack, BOOL *bEOT);

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


#endif // MPARSER_H
