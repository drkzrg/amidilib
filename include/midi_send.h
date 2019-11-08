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

static INLINE uint16 amMidiDataReady(uint8 deviceNo){
  return Bconstat(deviceNo);
}

static INLINE uint32 amMidiSendByte(uint8 deviceNo,uint8 data){
  return Bconout(deviceNo,(int16)data);
}

static INLINE void amMidiSendData(const uint16 count,const uint8 *data){
 //use xbios function
 Midiws(count,data);
 return; 
}

static INLINE uint8 amMidiGetData(uint8 deviceId){
  return (uint8)Bconin(deviceId); 
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

extern uint8 MIDIsendBuffer[32*1024]; //buffer from which we will send all data from the events once per frame
extern volatile uint16 MIDIbytesToSend;

#ifdef IKBD_MIDI_SEND_DIRECT
extern void clearMidiOutputBuffer();
extern void flushMidiSendBuffer();
#else
//clears custom midi output buffer
static void clearMidiOutputBuffer(){
    MIDIbytesToSend=0;
    amMemSet(MIDIsendBuffer,0,MIDI_SENDBUFFER_SIZE*sizeof(uint8));
}

static INLINE void flushMidiSendBuffer(){
    if(MIDIbytesToSend>0){
        amMidiSendData(MIDIbytesToSend,MIDIsendBuffer);
    }

    clearMidiOutputBuffer();
}
#endif

#ifdef DEBUG_BUILD
static void printMidiSendBufferState(){
    amTrace("Midi send buffer bytes to send: %d\n",MIDIbytesToSend);

    if(MIDIbytesToSend>0){
        int i=0;
        for(i;i<MIDIbytesToSend;++i){
            amTrace("0x%02x ",MIDIsendBuffer[i]);
        }

        amTrace(".\n");
    }
}
#endif

#endif
