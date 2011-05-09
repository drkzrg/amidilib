#ifndef __MIDI_SEND_H__
#define __MIDI_SEND_H__

#include "c_vars.h"

#ifndef PORTABLE

#include <mint/osbind.h>
#include "amlog.h"

//this is Atari specific
#define MIDI_FLOWCTRL 0		/* flow control enable/disable, default by: disabled*/
#define MIDI_LWM 32		/* low watermark if flow control enabled */
#define MIDI_HWM 32000		/* hight watermark if flow control enabled */
#define MIDI_BUFFER_SIZE 32767  /*default MIDI buffer size 32k */
#endif

//midi data sending, platform specific
static inline U16 amMidiDataReady(U8 deviceNo){
#ifdef PORTABLE
//TODO:
  amTrace((const U8)"WARNING: amMidiDataReady() not implemented\n");
  return 0;
#else
  return Bconstat(deviceNo);
#endif
}

static inline U32 amMidiSendByte(U8 deviceNo,U16 data){
#ifdef PORTABLE
//TODO:
  amTrace((const U8)"WARNING: amMidiSendByte() not implemented\n");
  return 0L;
#else
  return Bconout(deviceNo,data);
#endif
}

static inline void amMidiSendData(U16 count,U8 *data){
#ifdef PORTABLE
//TODO:
 amTrace((const U8)"WARNING: amMidiSendData() not implemented\n");
  return;
#else
  Midiws(count,data);
  return; 
#endif
}

static inline U8 amMidiGetData(U8 deviceId){
#ifdef PORTABLE
//TODO:
amTrace((const U8)"WARNING: amMidiGetData() not implemented\n");
  return 0;
#else
  return (U8)Bconin(deviceId); 
#endif
}

/* returns != 0 if data are in system MIDI buffer */
#define MIDI_DATA_READY amMidiDataReady(DEV_MIDI)
/* sends 1 byte to MIDI output */
#define MIDI_SEND_BYTE(data) amMidiSendByte(DEV_MIDI,(U8)data)
/* sends multiple bytes to MIDI output, count is number of bytes to send */
#define MIDI_SEND_DATA(count,data) amMidiSendData(count-1,data)
/* reads 1 unsigned byte from MIDI input */
#define GET_MIDI_DATA amMidiGetData(DEV_MIDI)


#endif