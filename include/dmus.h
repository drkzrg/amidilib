#ifndef __DMUS_H__
#define __DMUS_H__

/**  Copyright 2014 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/
#include <c_vars.h>

// mus header
typedef struct tagMUSheader_t {
  U32    ID;          	// identifier "MUS" 0x1A, 0x4D5553
  U16    scoreLen;
  U16    scoreStart;
  U16    channels;	// count of primary channels
  U16    sec_channels;	// count of secondary channels
  U16    instrCnt;
  U16    dummy;
  //// variable-length part starts here
} MUSheader_t ;

#define MUS_ID 0x4D5553


S32 Mus2Midi(U8* bytes, U8* out, const S8 *pOutMidName,U32* len);

#endif
