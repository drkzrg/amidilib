#ifndef __MIDI_SEND_H__
#define __MIDI_SEND_H__

#include "c_vars.h"
#include "amlog.h"

#include <mint/osbind.h>

#ifndef IKBD_MIDI_SEND_DIRECT
#define MIDI_FLOWCTRL 0		/* flow control enable/disable, default by: disabled*/
#define MIDI_LWM 32		/* low watermark if flow control enabled */
#define MIDI_HWM 32000		/* hight watermark if flow control enabled */
#endif

#define MIDI_SENDBUFFER_SIZE (32*1024) /*default MIDI buffer size 32k */
                                       // see common_m68k.inc

//midi data sending, platform specific

static INLINE U16 amMidiDataReady(U8 deviceNo){
  return Bconstat(deviceNo);
}

static INLINE U32 amMidiSendByte(U8 deviceNo,U8 data){
  return Bconout(deviceNo,(S16)data);
}

static INLINE void amMidiSendData(const U16 count,const U8 *data){
 //use xbios function
 Midiws(count,data);
 return; 
}

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

extern U8 MIDIsendBuffer[32*1024]; //buffer from which we will send all data from the events once per frame
extern U16 MIDIbytesToSend;

//clears custom midi output buffer
static void clearMidiOutputBuffer(){
    MIDIbytesToSend=0;
    amMemSet(MIDIsendBuffer,0,MIDI_SENDBUFFER_SIZE*sizeof(U8));
}

static INLINE void flushMidiSendBuffer(){
    if(MIDIbytesToSend>0){
        amMidiSendData(MIDIbytesToSend,MIDIsendBuffer);
    }

    clearMidiOutputBuffer();
}



#ifdef DEBUG_BUILD
static void printMidiSendBufferState(){
    amTrace("Midi send buffer bytes to send: %d\n",MIDIbytesToSend);

    if(MIDIbytesToSend>0){
        for(int i=0;i<MIDIbytesToSend;++i){
            amTrace("0x%02x ",MIDIsendBuffer[i]);
        }

        amTrace(".\n");
    }
}
#endif

#endif
