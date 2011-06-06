
/**  Copyright 2007-2010 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#include "include/midi_send.h"

//midi data sending, platform specific
 U16 amMidiDataReady(U8 deviceNo){
#ifdef PORTABLE
//TODO:
  amTrace((const U8*)"WARNING: amMidiDataReady() not implemented\n");
  return 0;
#else
  return Bconstat(deviceNo);
#endif
}

 U32 amMidiSendByte(U8 deviceNo,U16 data){
#ifdef PORTABLE
//TODO:
  amTrace((const U8*)"WARNING: amMidiSendByte() not implemented\n");
  return 0L;
#else
  return Bconout(deviceNo,data);
#endif
}

 void amMidiSendData(const U16 count,const U8 *data){
#ifdef PORTABLE
//TODO:
 amTrace((const U8*)"WARNING: amMidiSendData() not implemented\n");
 return;
#else
  Midiws(count,data);
  return; 
#endif
}

 U8 amMidiGetData(U8 deviceId){
#ifdef PORTABLE
//TODO:
amTrace((const U8*)"WARNING: amMidiGetData() not implemented\n");
  return 0;
#else
  return (U8)Bconin(deviceId); 
#endif
}
