
#include "include/midi_send.h"

//midi data sending, platform specific
extern inline U16 amMidiDataReady(U8 deviceNo);
extern inline void amMidiSendByte(U8 deviceNo,U8 data);
extern inline void amMidiSendData(U16 count,U8 *data);
extern inline U8 amMidiGetData(U8 deviceId);

