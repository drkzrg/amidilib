#ifndef __DMUS_H__
#define __DMUS_H__

/**  Copyright 2007-2021 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/
#include "vartypes.h"

// mus header
typedef struct tagMUSheader_t {
  uint32    ID;          	// identifier "MUS" 0x1A, 0x4D5553
  uint16    scoreLen;
  uint16    scoreStart;
  uint16    channels;	// count of primary channels
  uint16    sec_channels;	// count of secondary channels
  uint16    instrCnt;
  uint16    dummy;
  //// variable-length part starts here
} MUSheader_t ;

#define MUS_ID 0x4D5553


int16 Mus2Midi(uint8* bytes, uint8* out, const int8 *pOutMidName,uint32* len);

#endif
