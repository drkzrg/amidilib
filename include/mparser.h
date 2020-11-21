#ifndef MPARSER_H
#define MPARSER_H

/** main midi parser
    parses midi data and transforms it to custom sequence
*/

/**  Copyright 2007-2020 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#include "c_vars.h"
#include "amidiseq.h"
#include "mformats.h"

/**
 * gets time division from MIDI header data
 *
 * @param pMidi	memory pointer where (X)Midi file is stored.
 * @return number of tracks in loaded MIDI file, -1 if error occured
 */

uint16 amGetTimeDivision(const void *pMidiPtr);

/**
* Reads Variable lenght value used in MIDI files. Returns value in Variable lenght format
* @param none
**/
extern uint32 amReadVarLen(void);

/**
 * Returns MIDI file type of stored Midi data
 *
 * @param pMidiPtr	address wheere midi file data is stored
 * @return (0)- MIDI file type, see eMidiFileType enum for possible values
 */

eMidiFileType amGetMidiDataType( void *const pMidiPtr);

/** processes the MIDI 0,1,2 track data
*	@param trackStartPtr pointer to the start of Track MIDI chunk
*	@param fileTypeFlag kind of file to preprocess
*	@param ppCurSequence pointer a pointer to AMIDI sequence structure
*	@param iRetVal pointer to an integer, which holds operation status after events processing (AM_OK on success, AM_ERR on error)
*	@return pointer to the next chunk or NULL if EOT occured.
*/
void *processMidiTracks(void *trackStartPtr, const eMidiFileType fileTypeFlag, sSequence_t **ppCurSequence, retVal *iRetVal);

/** processes the MIDI 0,1,2 track events
*	@param pSeq pointer to AMIDI sequence structure
*	@param startPtr pointer to an address containing start of Track MIDI chunk
*	@param endAddr address, where track data ends
*	@param trackNb track number to process (from 0 to 15)
*	@param iRetVal pointer to an integer, which holds operation status after events processing (AM_OK on success, AM_ERR on error)
*	@return pointer to the next chunk or NULL if EOT occured.
    TODO: inline it ?
*/

void *processMidiTrackEvents(sSequence_t *pSeq, void** startPtr, const void *endAddr, const uint8 trackNb, retVal *iRetVal);

/** read/decode note off message
*	@param pSeq pointer to AMIDI sequence structure
*   @param pPtr pointer to an address pointing to start of midi data to read
*   @param recallRS current recall running status
*   @param delta current event delta
*   @param trackNb index of track structure, where event will be stored
*	@return iRetVal pointer to an integer, which holds operation status after events processing (AM_OK on success, AM_ERR on error)
*/
retVal amNoteOff(sSequence_t *pSeq, uint8 **pPtr,uint16 *recallRS, const uint32 delta, const uint8 trackNb);

/** read/decode note on message
*	@param pSeq pointer to AMIDI sequence structure
*   @param pPtr pointer to an address pointing to start of midi data to read
*   @param recallRS current recall running status
*   @param delta current event delta
*   @param trackNb index of track structure, where event will be stored
*	@return iRetVal pointer to an integer, which holds operation status after events processing (AM_OK on success, AM_ERR on error)
*/
retVal amNoteOn(sSequence_t *pSeq, uint8 **pPtr,uint16 *recallRS, const uint32 delta, const uint8 trackNb);

/** read/decode note aftertouch/pressure message
*	@param pSeq pointer to AMIDI sequence structure
*   @param pPtr pointer to an address pointing to start of midi data to read
*   @param recallRS current recall running status
*   @param delta current event delta
*   @param trackNb index of track structure, where event will be stored
*	@return iRetVal pointer to an integer, which holds operation status after events processing (AM_OK on success, AM_ERR on error)
*/
retVal amNoteAft(sSequence_t *pSeq, uint8 **pPtr,uint16 *recallRS, const uint32 delta, const uint8 trackNb);

/** read/decode control change message
*	@param pSeq pointer to AMIDI sequence structure
*   @param pPtr pointer to an address pointing to start of midi data to read
*   @param recallRS current recall running status
*   @param delta current event delta
*   @param trackNb index of track structure, where event will be stored
*	@return iRetVal pointer to an integer, which holds operation status after events processing (AM_OK on success, AM_ERR on error)
*/
retVal amController(sSequence_t *pSeq, uint8 **pPtr, uint16 *recallRS, const uint32 delta, const uint8 trackNb);

/** read/decode program change message
*	@param pSeq pointer to AMIDI sequence structure
*   @param pPtr pointer to an address pointing to start of midi data to read
*   @param recallRS current recall running status
*   @param delta current event delta
*   @param trackNb index of track structure, where event will be stored
*	@return iRetVal pointer to an integer, which holds operation status after events processing (AM_OK on success, AM_ERR on error)
*/
retVal amProgramChange(sSequence_t *pSeq, uint8 **pPtr,uint16 *recallRS, const uint32 delta, const uint8 trackNb);

/** read/decode channel aftertouch/pressure message
*	@param pSeq pointer to AMIDI sequence structure
*   @param pPtr pointer to an address pointing to start of midi data to read
*   @param recallRS current recall running status
*   @param delta current event delta
*   @param trackNb index of track structure, where event will be stored
*	@return iRetVal pointer to an integer, which holds operation status after events processing (AM_OK on success, AM_ERR on error)
*/
retVal amChannelAft(sSequence_t *pSeq, uint8 **pPtr,uint16 *recallRS, const uint32 delta, const uint8 trackNb);

/** read/decode pitch bend message
*	@param pSeq pointer to AMIDI sequence structure
*   @param pPtr pointer to an address pointing to start of midi data to read
*   @param recallRS current recall running status
*   @param delta current event delta
*   @param trackNb index of track structure, where event will be stored
*	@return iRetVal pointer to an integer, which holds operation status after events processing (AM_OK on success, AM_ERR on error)
*/
retVal amPitchBend(sSequence_t *pSeq, uint8 **pPtr,uint16 *recallRS, const uint32 delta, const uint8 trackNb);

/** read/decode sysex message
*	@param pSeq pointer to AMIDI sequence structure
*   @param pPtr pointer to an address pointing to start of midi data to read
*   @param recallRS current recall running status
*   @param delta current event delta
*   @param trackNb index of track structure, where event will be stored
*	@return iRetVal pointer to an integer, which holds operation status after events processing (AM_OK on success, AM_ERR on error)
*/
retVal amSysexMsg(sSequence_t *pSeq, uint8 **pPtr, const uint32 delta, const uint8 trackNb);

/** read/decode meta event message
*	@param pSeq pointer to AMIDI sequence structure
*   @param pPtr pointer to an address pointing to start of midi data to read
*   @param recallRS current recall running status
*   @param delta current event delta
*   @param trackNb index of track structure, where event will be stored
*   @return Bool flag TRUE if end of track (EOT) occured, FALSE otherwise. iRetVal pointer to an integer, 
*           which holds operation status after events processing (AM_OK on success, AM_ERR on error)
*/
retVal amMetaEvent(sSequence_t *pSeq, uint8 **pPtr, const uint32 delta, const uint8 trackNb, Bool *bEOT);

/** checks if byte is midi channel
*	@param byteEvent command byte
*	@return 1 if true, 0 otherwise
*/
Bool amIsMidiChannelEvent(const uint8 byteEvent);

/** checks if byte is Sysex or System realtime command byte
*	@param byteEvent pointer to VLQ data
*	@return 1 if true, 0 otherwise
*/
Bool amIsMidiRtCmdOrSysex(const uint8 byteEvent);

/* combine bytes function (14 bit values) for pitch bend */
uint16 amCombinePitchBendBytes(const uint8 bFirst, const uint8 bSecond);

#endif // MPARSER_H
