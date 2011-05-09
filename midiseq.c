
/**  Copyright 2007-2010 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#include "include/midiseq.h"

//inline functions for sending data to external module
extern inline const U8 *getEventName(U32 id);

extern inline void  fNoteOn (void *pEvent);
extern inline void  fNoteOff (void *pEvent);
extern inline void  fNoteAft (void *pEvent);
extern inline void  fProgramChange (void *pEvent);
extern inline void  fController (void *pEvent);
extern inline void  fChannelAft (void *pEvent);
extern inline void  fPitchBend (void *pEvent);
extern inline void  fSetTempo (void *pEvent);

/* returns the info struct about event: size and pointer to the handler  */
extern inline void getEventFuncInfo(U8 eventType, sEventInfoBlock_t *infoBlk);