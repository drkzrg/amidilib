
/**  Copyright 2007-2021 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#ifndef AXMIDI_H
#define AXMIDI_H

#include "vartypes.h"
#include "amidiseq.h"

/**
 * Checks if xmidi file is valid 
 *
 * @param xmidiData	- pointer to xmidi data
 * @return returns true if data are correct. 
 **/

Bool amIsValidXmidiData(void *xmidiData);

/**
 * Gets number of sequences/tracks stored in xmidi data 
 *
 * @param xmidiData	- pointer to xmidi data
 * @return returns number of tracks
 **/

uint16 amGetNbOfXmidiTracks(void *midiData);


/**
 * Transforms XMidi data to Amidilib sequence 
 *
 * @param xmidiData	- pointer to xmidi data
 * @return returns value AM_OK on success, AM_ERR on error
 **/

int16 amProcessXmidiData(void *xmidiData, const uint32 dataLength, sSequence_t **ppCurSequence);

#endif
