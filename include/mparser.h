#ifndef MPARSER_H
#define MPARSER_H

/** main midi parser
    parses midi data and transforms it to custom sequence
*/

/**  Copyright 2007-2021 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#include "vartypes.h"
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

/* combine bytes function (14 bit values) for pitch bend */
uint16 amCombinePitchBendBytes(const uint8 bFirst, const uint8 bSecond);

#endif // MPARSER_H
