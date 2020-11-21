
#ifndef _NKT_UTIL_H
#define _NKT_UTIL_H

/**  Copyright 2007-2020 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/


#include "c_vars.h"
#include "nkt.h"
#include "midi.h"

// various utility functions
// not exposed in main interface

// struct for collecting info about midi track
// to figure out how much space we will ned for event blocks and data buffers
// we need info about overall number of blocks and buffer size for data storage

typedef struct sMidiTrackInfo{
 uint32 nbOfBlocks;        // nb of event blocks
 uint32 bufPos;            // dummy buffer writing position
                        // cleared after midi event data block
 uint32 eventsBlockSize;
 uint32 dataBlockSize;

} sMidiTrackInfo_t;

typedef struct sRunningStatus{
  uint16 recallRS;
  uint8 runningStatus;
} sRunningStatus_t;


int32 saveSequence(sNktSeq *pSeq,const uint8 *filepath, Bool bCompressed);

#endif
