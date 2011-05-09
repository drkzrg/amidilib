/**  Copyright 2007-2010 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#include "include/midi_send.h"

//midi data sending, platform specific
extern inline U16 amMidiDataReady(U8 deviceNo);
extern inline U32 amMidiSendByte(U8 deviceNo,U16 data);
extern inline void amMidiSendData(U16 count,U8 *data);
extern inline U8 amMidiGetData(U8 deviceId);

