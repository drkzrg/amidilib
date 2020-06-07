
/**  Copyright 2007-2020 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#ifndef AXMIDI_H
#define AXMIDI_H

#include "c_vars.h"
#include "amidiseq.h"

bool isValidXmidiData(void *xmidiData);
uint16 processXmidiData(void *xmidiData, const uint32 dataLength, sSequence_t **ppCurSequence);

#endif
