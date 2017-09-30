
#ifndef _NKT_UTIL_H
#define _NKT_UTIL_H

/**  Copyright 2007-2015 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
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
 U32 nbOfBlocks;        // nb of event blocks
 U32 bufPos;            // dummy buffer writing position
                        // cleared after midi event data block
 U32 eventsBlockSize;
 U32 dataBlockSize;

} sMidiTrackInfo_t;

typedef struct sRunningStatus{
  U16 recallRS;
  U8 runningStatus;
} sRunningStatus_t;


S32 saveSequence(sNktSeq *pSeq,const U8 *filepath,BOOL bCompressed);

#endif
