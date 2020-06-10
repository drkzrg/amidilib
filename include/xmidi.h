
/**  Copyright 2007-2020 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#ifndef AXMIDI_H
#define AXMIDI_H

#include "c_vars.h"
#include "amidiseq.h"

bool amIsValidXmidiData(void *xmidiData);
uint16 amGetNbOfXmidiTracks(void *midiData);
uint16 amProcessXmidiData(void *xmidiData, const uint32 dataLength, sSequence_t **ppCurSequence);

#endif
