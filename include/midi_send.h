#ifndef __MIDI_SEND_H__
#define __MIDI_SEND_H__

#include "c_vars.h"
#include "amlog.h"

#include <mint/osbind.h>

#define MIDI_FLOWCTRL 0		/* flow control enable/disable, default by: disabled*/
#define MIDI_LWM 32		/* low watermark if flow control enabled */
#define MIDI_HWM 32000		/* hight watermark if flow control enabled */
#define MIDI_BUFFER_SIZE 32767  /*default MIDI buffer size 32k */

#ifdef IKBD_MIDI_SEND_DIRECT  
#define MIDI_SENDBUFFER_SIZE (32*1024) //see common_m68k.inc
  //bypass of Atari XBIOS, writes directly to IKBD to send data
  extern void amMidiSendIKBD();  
#endif


//midi data sending, platform specific
static INLINE U16 amMidiDataReady(U8 deviceNo){
  return Bconstat(deviceNo);

}

#ifndef IKBD_MIDI_SEND_DIRECT  
static INLINE U32 amMidiSendByte(U8 deviceNo,U8 data){
  return Bconout(deviceNo,(S16)data);
}

static INLINE void amMidiSendData(const U16 count,const U8 *data){
 //use xbios function
 Midiws(count,data);
 return; 
}
#else
 //bypass os, write directly to IKBD to send data
 //we do nothing here, midi data buffer will be emptied in interrupt routine
 //if we use MIDI_SEND_DATA manually we need to call amMidiSendIKBD() function.
#endif //IKBD_MIDI_SEND_DIRECT


static INLINE U8 amMidiGetData(U8 deviceId){
  return (U8)Bconin(deviceId); 
} 

/* returns != 0 if data are in system MIDI buffer */
#define MIDI_DATA_READY amMidiDataReady(DEV_MIDI)

#ifndef IKBD_MIDI_SEND_DIRECT 
/* sends 1 byte to MIDI output */
#define MIDI_SEND_BYTE(data) amMidiSendByte(DEV_MIDI,data)
/* sends multiple bytes to MIDI output, count is number of bytes to send */
#define MIDI_SEND_DATA(count,data) amMidiSendData(count-1,data)
#endif

/* reads 1 unsigned byte from MIDI input */
#define GET_MIDI_DATA amMidiGetData(DEV_MIDI)


#endif
