
/**  Copyright 2007-2010 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/


#ifndef __MIDI_CMD_H__
#define __MIDI_CMD_H__

#include "c_vars.h"
#include "memory/memory.h"
#include "midi_send.h"
#include "events.h"

#define MIDI_MASTER_VOL_MAX    127
#define MIDI_MASTER_VOL_MIN    0

#define MIDI_MASTER_PAN_MAX    127
#define MIDI_MASTER_PAN_CENTER 64
#define MIDI_MASTER_PAN_MIN    0

#define MIDI_DEFAULT_MVOL   (127/2)

#ifndef IKBD_MIDI_SEND_DIRECT
/* small static buffer for sending MIDI commands */
static U8 g_midi_cmd_buffer[4];
#endif


typedef struct SysEX_t{
   U16 size;
   U8 *data;
}sSysEX_t;

//////////////////////////////////////////////////////////////////////////////
// helper functions for copying midi data to internal buffer
extern volatile U8 MIDIsendBuffer[32*1024]; //buffer from which we will send all data from the events once per frame
extern volatile U16 MIDIbytesToSend;

// sends SysEX message without recalculating the checksum
static INLINE void sendSysEX(const sSysEX_t *pMsg){

amTrace("Send SysEx size: %lu \n",pMsg->size);

#ifdef IKBD_MIDI_SEND_DIRECT
 amMemCpy(&MIDIsendBuffer[MIDIbytesToSend],pMsg->data,pMsg->size);
 MIDIbytesToSend+=pMsg->size;

 Supexec(flushMidiSendBuffer);
#else
    MIDI_SEND_DATA(pMsg->size,pMsg->data);
#endif
}

/* common, channel voice messages */
/**
 * sends NOTE OFF MIDI message (key depressed)
 *
 * @param channel MIDI channel number 1-16 (0x00-0x0f).
 * @param note specifies which note to play 0-127 (0x00-0x7F)
 * @param velocity with what velocity 0-127 (0x00-0x7F)
 */

static INLINE void note_off (const U8 channel,const U8 note,const U8 velocity ){
#ifdef IKBD_MIDI_SEND_DIRECT
	MIDIsendBuffer[MIDIbytesToSend++]=(EV_NOTE_OFF<<4)|(channel);
	MIDIsendBuffer[MIDIbytesToSend++]=note;
	MIDIsendBuffer[MIDIbytesToSend++]=velocity;
#else
	g_midi_cmd_buffer[0]=(EV_NOTE_OFF<<4)|(channel);
	g_midi_cmd_buffer[1]=note;
	g_midi_cmd_buffer[2]=velocity;

	MIDI_SEND_DATA(3,g_midi_cmd_buffer);
#endif	
}

/**
 * sends NOTE ON MIDI message (key pressed)
 *
 * @param channel MIDI channel number 1-16 (0x00-0x0f).
 * @param note specifies which note to play 0-127 (0x00-0x7F)
 * @param velocity with what velocity 0-127 (0x00-0x7F)
 */
static INLINE void note_on (const U8 channel,const U8 note, const U8 velocity){
#ifdef IKBD_MIDI_SEND_DIRECT
	MIDIsendBuffer[MIDIbytesToSend++]=(EV_NOTE_ON<<4)|(channel);
	MIDIsendBuffer[MIDIbytesToSend++]=note;
	MIDIsendBuffer[MIDIbytesToSend++]=velocity;
#else
	g_midi_cmd_buffer[0]=(EV_NOTE_ON<<4)|channel;
	g_midi_cmd_buffer[1]=note;
	g_midi_cmd_buffer[2]=velocity;
	MIDI_SEND_DATA(3,g_midi_cmd_buffer);
#endif
}

/**
 * sends POLYPHONIC KEY PRESSURE MIDI message (note aftertouch)
 *
 * @param channel MIDI channel number 1-16 (0x00-0x0f).
 * @param note specifies which note to play 0-127 (0x00-0x7F)
 * @param value value 0-127 (0x00-0x7F)
 */

static INLINE void polyphonic_key_press(const U8 channel,const U8 note,const U8 value){
#ifdef IKBD_MIDI_SEND_DIRECT
	MIDIsendBuffer[MIDIbytesToSend++]=(EV_NOTE_AFTERTOUCH<<4)|channel;
	MIDIsendBuffer[MIDIbytesToSend++]=note;
	MIDIsendBuffer[MIDIbytesToSend++]=value;
#else
	g_midi_cmd_buffer[0]=(EV_NOTE_AFTERTOUCH<<4)|channel;
	g_midi_cmd_buffer[1]=note;
	g_midi_cmd_buffer[2]=value;
	MIDI_SEND_DATA(3,g_midi_cmd_buffer);
#endif
}
/**
 * sends CONTROL CHANGE MIDI message
 * @param controller specifies which controller to use, full list is in EVENTS.H (from midi specification)
 * and additional for XMIDI are in XMIDI.H. Value can be 0-127 (0x00-0x7F)
 * @param channel MIDI channel number 1-16 (0x00-0x0f).
 * @param value value sent to given controller 0-127 (0x00-0x7F)
 */

static INLINE void control_change(const U8 controller,const U8 channel,const U8 value1,const U8 value2){
#ifdef IKBD_MIDI_SEND_DIRECT
	MIDIsendBuffer[MIDIbytesToSend++]=(EV_CONTROLLER<<4)|channel;
	MIDIsendBuffer[MIDIbytesToSend++]=controller;
	MIDIsendBuffer[MIDIbytesToSend++]=value1;
	MIDIsendBuffer[MIDIbytesToSend++]=value2;
#else
	g_midi_cmd_buffer[0]=(EV_CONTROLLER<<4)|channel;
	g_midi_cmd_buffer[1]=controller;
	g_midi_cmd_buffer[2]=value1;
	g_midi_cmd_buffer[3]=value2;
	
	MIDI_SEND_DATA(4,g_midi_cmd_buffer);
#endif 
}

/**
 * sends PROGRAM CHANGE MIDI message
 * @param channel MIDI channel number 1-16 (0x00-0x0f).
 * @param programNb program number 1-128 (0x00-0x7F)
 */

static INLINE void program_change(const U8 channel,const U8 programNb){
#ifdef IKBD_MIDI_SEND_DIRECT
	MIDIsendBuffer[MIDIbytesToSend++]=(EV_PROGRAM_CHANGE<<4)|channel;
	MIDIsendBuffer[MIDIbytesToSend++]=programNb;
#else
	g_midi_cmd_buffer[0]=(EV_PROGRAM_CHANGE<<4)|channel;
	g_midi_cmd_buffer[1]=programNb;
	MIDI_SEND_DATA(2,g_midi_cmd_buffer);
#endif	
}

/**
 * sends CHANNEL PRESSURE MIDI message
 * @param channel MIDI channel number 1-16 (0x00-0x0f).
 * @param value program number 0-127 (0x00-0x7F)
 * @remarks valid only for GS sound source
 */

static INLINE void channel_pressure (const U8 channel,const U8 value){
#ifdef IKBD_MIDI_SEND_DIRECT
	MIDIsendBuffer[MIDIbytesToSend++]=(EV_CHANNEL_AFTERTOUCH<<4)|channel;
	MIDIsendBuffer[MIDIbytesToSend++]=value;
#else
	g_midi_cmd_buffer[0]=(EV_CHANNEL_AFTERTOUCH<<4)|channel;
	g_midi_cmd_buffer[1]=value;
	MIDI_SEND_DATA(2,g_midi_cmd_buffer);
#endif
}

/**
 * sends PITCH BEND MIDI message
 * @param channel MIDI channel number 1-16 (0x00-0x0f).
 * @param value signed U16 value -8192 - 0 - +8191 (0x0000 - 0x4000 - 0x7F7F)
 */
static INLINE void pitch_bend (const U8 channel,const U16 value){
#ifdef IKBD_MIDI_SEND_DIRECT
	MIDIsendBuffer[MIDIbytesToSend++]=(EV_PITCH_BEND<<4)|channel;
	MIDIsendBuffer[MIDIbytesToSend++]=(U8)((value>>4)&0x0F);
	MIDIsendBuffer[MIDIbytesToSend++]=(U8)(value&0x0F);
#else
	g_midi_cmd_buffer[0]=(EV_PITCH_BEND<<4)|channel;
	g_midi_cmd_buffer[1]=(U8)((value>>4)&0x0F);
	g_midi_cmd_buffer[2]=(U8)(value&0x0F);
	MIDI_SEND_DATA(3,g_midi_cmd_buffer);
#endif
}

/**
 * sends PITCH BEND MIDI message
 * @param channel MIDI channel number 1-16 (0x00-0x0f).
 * @param fbyte U8 value which gives with sbyte S16 value with range: -8192 - 0 - +8191 (0x0000 - 0x4000 - 0x7F7F)
 * @param sbyte U8 value which gives with fbyte S16 value with range: -8192 - 0 - +8191 (0x0000 - 0x4000 - 0x7F7F)
 */

static INLINE void pitch_bend_2 (const U8 channel, const U8 fbyte,const U8 sbyte){
#ifdef IKBD_MIDI_SEND_DIRECT
	MIDIsendBuffer[MIDIbytesToSend++]=(EV_PITCH_BEND<<4)|channel;
	MIDIsendBuffer[MIDIbytesToSend++]=fbyte;
	MIDIsendBuffer[MIDIbytesToSend++]=sbyte;
#else
	g_midi_cmd_buffer[0]=(EV_PITCH_BEND<<4)|channel;
	g_midi_cmd_buffer[1]=fbyte;
	g_midi_cmd_buffer[2]=sbyte;
	MIDI_SEND_DATA(3,g_midi_cmd_buffer);
#endif	
}

/* channel mode messages */
/**
 * sends ALL SOUNDS OFF MIDI message
 * @param channel MIDI channel number 1-16 (0x00-0x0f).
 */

static INLINE void all_sounds_off(const U8 channel){
#ifdef IKBD_MIDI_SEND_DIRECT
	MIDIsendBuffer[MIDIbytesToSend++]=(EV_CONTROLLER<<4)|channel;
	MIDIsendBuffer[MIDIbytesToSend++]=C_SOUNDS_OFF;
	MIDIsendBuffer[MIDIbytesToSend++]=0x00;
#else

	g_midi_cmd_buffer[0]=(EV_CONTROLLER<<4)|channel;
	g_midi_cmd_buffer[1]=C_SOUNDS_OFF;
	g_midi_cmd_buffer[2]=0x00;

	MIDI_SEND_DATA(3,g_midi_cmd_buffer);
#endif
}

/**
 * sends RESET ALL CONTROLLERS MIDI message
 * @param channel MIDI channel number 1-16 (0x00-0x0f).
 */
static INLINE void reset_all_controllers(const U8 channel){
#ifdef IKBD_MIDI_SEND_DIRECT
	MIDIsendBuffer[MIDIbytesToSend++]=(EV_CONTROLLER<<4)|channel;
	MIDIsendBuffer[MIDIbytesToSend++]=C_RESET_ALL;
	MIDIsendBuffer[MIDIbytesToSend++]=0x00;
#else
	g_midi_cmd_buffer[0]=(EV_CONTROLLER<<4)|channel;
	g_midi_cmd_buffer[1]=C_RESET_ALL;
	g_midi_cmd_buffer[2]=0x00;

	MIDI_SEND_DATA(3,g_midi_cmd_buffer);
#endif	
}

/**
 * sends ALL NOTES OFF MIDI message
 * @param channel MIDI channel number 1-16 (0x00-0x0f).
 */
static INLINE void all_notes_off(const U8 channel){
#ifdef IKBD_MIDI_SEND_DIRECT
	MIDIsendBuffer[MIDIbytesToSend++]=(EV_CONTROLLER<<4)|channel;
	MIDIsendBuffer[MIDIbytesToSend++]=C_ALL_NOTES_OFF;
	MIDIsendBuffer[MIDIbytesToSend++]=0x00;
#else
	g_midi_cmd_buffer[0]=(EV_CONTROLLER<<4)|channel;
	g_midi_cmd_buffer[1]=C_ALL_NOTES_OFF;
	g_midi_cmd_buffer[2]=0x00;

	MIDI_SEND_DATA(3,g_midi_cmd_buffer);
#endif	
}

/**
 * sends OMNI OFF MIDI message
 * @param channel MIDI channel number 1-16 (0x00-0x0f).
 */
static INLINE void omni_off(const U8 channel){
#ifdef IKBD_MIDI_SEND_DIRECT
	MIDIsendBuffer[MIDIbytesToSend++]=(EV_CONTROLLER<<4)|channel;
	MIDIsendBuffer[MIDIbytesToSend++]=C_OMNI_OFF;
	MIDIsendBuffer[MIDIbytesToSend++]=0x00;
#else
	g_midi_cmd_buffer[0]=(EV_CONTROLLER<<4)|channel;
	g_midi_cmd_buffer[1]=C_OMNI_OFF;
	g_midi_cmd_buffer[2]=0x00;
	MIDI_SEND_DATA(3,g_midi_cmd_buffer);
#endif	
}

/**
 * sends OMNI ON MIDI message
 * @param channel MIDI channel number 1-16 (0x00-0x0f).
 */
static INLINE void omni_on(const U8 channel){
#ifdef IKBD_MIDI_SEND_DIRECT
	MIDIsendBuffer[MIDIbytesToSend++]=(EV_CONTROLLER<<4)|channel;
	MIDIsendBuffer[MIDIbytesToSend++]=C_OMNI_ON;
	MIDIsendBuffer[MIDIbytesToSend++]=0x00;
#else
	g_midi_cmd_buffer[0]=(EV_CONTROLLER<<4)|channel;
	g_midi_cmd_buffer[1]=C_OMNI_ON;
	g_midi_cmd_buffer[2]=0x00;
	MIDI_SEND_DATA(3,g_midi_cmd_buffer);
#endif	
}

/**
 * sends MONO MIDI message
 * @param channel MIDI channel number 1-16 (0x00-0x0f).
 */
static INLINE void mono_mode(const U8 channel,const U8 numberOfMono){
#ifdef IKBD_MIDI_SEND_DIRECT
	MIDIsendBuffer[MIDIbytesToSend++]=(EV_CONTROLLER<<4)|channel;
	MIDIsendBuffer[MIDIbytesToSend++]=C_MONO;
	MIDIsendBuffer[MIDIbytesToSend++]=numberOfMono;
#else
	g_midi_cmd_buffer[0]=(EV_CONTROLLER<<4)|channel;
	g_midi_cmd_buffer[1]=C_MONO;
	g_midi_cmd_buffer[2]=numberOfMono;
	MIDI_SEND_DATA(3,g_midi_cmd_buffer);
#endif	
}

/**
 * sends POLY MIDI message
 * @param channel MIDI channel number 1-16 (0x00-0x0f).
 */

static INLINE void poly_mode(const U8 channel,const U8 numberOfPoly){
#ifdef IKBD_MIDI_SEND_DIRECT
	MIDIsendBuffer[MIDIbytesToSend++]=(EV_CONTROLLER<<4)|channel;
	MIDIsendBuffer[MIDIbytesToSend++]=C_POLY;
	MIDIsendBuffer[MIDIbytesToSend++]=numberOfPoly;
#else
	g_midi_cmd_buffer[0]=(EV_CONTROLLER<<4)|channel;
	g_midi_cmd_buffer[1]=C_POLY;
	g_midi_cmd_buffer[2]=numberOfPoly;
	MIDI_SEND_DATA(3,g_midi_cmd_buffer);
#endif	
}

/* common, channel voice messages */
/**
 * copies NOTE OFF MIDI message (key depressed)
 *
 * @param channel MIDI channel number 1-16 (0x00-0x0f).
 * @param note specifies which note to play 0-127 (0x00-0x7F)
 * @param velocity with what velocity 0-127 (0x00-0x7F)
 */

static INLINE void copy_note_off (const U8 channel,const U8 note,const U8 velocity ){
	MIDIsendBuffer[MIDIbytesToSend++]=(EV_NOTE_OFF<<4)|(channel);
	MIDIsendBuffer[MIDIbytesToSend++]=note;
	MIDIsendBuffer[MIDIbytesToSend++]=velocity;
}

/**
 * copies NOTE ON MIDI message (key pressed)
 *
 * @param channel MIDI channel number 1-16 (0x00-0x0f).
 * @param note specifies which note to play 0-127 (0x00-0x7F)
 * @param velocity with what velocity 0-127 (0x00-0x7F)
 */

static INLINE void copy_note_on (const U8 channel,const U8 note,const U8 velocity){
	MIDIsendBuffer[MIDIbytesToSend++]=(EV_NOTE_ON<<4)|channel;
	MIDIsendBuffer[MIDIbytesToSend++]=note;
	MIDIsendBuffer[MIDIbytesToSend++]=velocity;
}

/**
 * copies POLYPHONIC KEY PRESSURE MIDI message (note aftertouch)
 *
 * @param channel MIDI channel number 1-16 (0x00-0x0f).
 * @param note specifies which note to play 0-127 (0x00-0x7F)
 * @param value value 0-127 (0x00-0x7F)
 */

static INLINE void copy_polyphonic_key_press(const U8 channel,const U8 note,const U8 value){
	MIDIsendBuffer[MIDIbytesToSend++]=(EV_NOTE_AFTERTOUCH<<4)|channel;
	MIDIsendBuffer[MIDIbytesToSend++]=note;
	MIDIsendBuffer[MIDIbytesToSend++]=value;
}
/**
 * copies CONTROL CHANGE MIDI message
 * @param controller specifies which controller to use, full list is in EVENTS.H (from midi specification)
 * and additional for XMIDI are in XMIDI.H. Value can be 0-127 (0x00-0x7F)
 * @param channel MIDI channel number 1-16 (0x00-0x0f).
 * @param value value sent to given controller 0-127 (0x00-0x7F)
 */

static INLINE void copy_control_change(const U8 controller,const U8 channel,const U8 value1, const U8 value2){
	MIDIsendBuffer[MIDIbytesToSend++]=(EV_CONTROLLER<<4)|channel;
	MIDIsendBuffer[MIDIbytesToSend++]=controller;
	MIDIsendBuffer[MIDIbytesToSend++]=value1;
	MIDIsendBuffer[MIDIbytesToSend++]=value2;
}

/**
 * copies PROGRAM CHANGE MIDI message
 * @param channel MIDI channel number 1-16 (0x00-0x0f).
 * @param programNb program number 1-128 (0x00-0x7F)
 */

static INLINE void copy_program_change(const U8 channel, const U8 programNb){
	MIDIsendBuffer[MIDIbytesToSend++]=(EV_PROGRAM_CHANGE<<4)|channel;
	MIDIsendBuffer[MIDIbytesToSend++]=programNb;
}

/**
 * copies CHANNEL PRESSURE MIDI message
 * @param channel MIDI channel number 1-16 (0x00-0x0f).
 * @param value program number 0-127 (0x00-0x7F)
 * @remarks valid only for GS sound source
 */

static INLINE void copy_channel_pressure (const U8 channel, const U8 value){
	MIDIsendBuffer[MIDIbytesToSend++]=(EV_CHANNEL_AFTERTOUCH<<4)|channel;
	MIDIsendBuffer[MIDIbytesToSend++]=value;
}

/**
 * copies PITCH BEND MIDI message
 * @param channel MIDI channel number 1-16 (0x00-0x0f).
 * @param value signed U16 value -8192 - 0 - +8191 (0x0000 - 0x4000 - 0x7F7F)
 */
static INLINE void copy_pitch_bend (const U8 channel,const U16 value){
	MIDIsendBuffer[MIDIbytesToSend++]=(EV_PITCH_BEND<<4)|channel;
	MIDIsendBuffer[MIDIbytesToSend++]=(U8)((value>>4)&0x0F);
	MIDIsendBuffer[MIDIbytesToSend++]=(U8)(value&0x0F);
}

/**
 * copies PITCH BEND MIDI message
 * @param channel MIDI channel number 1-16 (0x00-0x0f).
 * @param fbyte U8 value which gives with sbyte S16 value with range: -8192 - 0 - +8191 (0x0000 - 0x4000 - 0x7F7F)
 * @param sbyte U8 value which gives with fbyte S16 value with range: -8192 - 0 - +8191 (0x0000 - 0x4000 - 0x7F7F)
 */

static INLINE void copy_pitch_bend_2 (const U8 channel, const U8 fbyte,const U8 sbyte)
{
	MIDIsendBuffer[MIDIbytesToSend++]=(EV_PITCH_BEND<<4)|channel;
	MIDIsendBuffer[MIDIbytesToSend++]=fbyte; //LSB
	MIDIsendBuffer[MIDIbytesToSend++]=sbyte; //MSB
}

/* channel mode messages */
/**
 * copy ALL SOUNDS OFF MIDI message
 * @param channel MIDI channel number 1-16 (0x00-0x0f).
 */

static INLINE void copy_all_sounds_off(const U8 channel){
	MIDIsendBuffer[MIDIbytesToSend++]=(EV_CONTROLLER<<4)|channel;
	MIDIsendBuffer[MIDIbytesToSend++]=C_SOUNDS_OFF;
	MIDIsendBuffer[MIDIbytesToSend++]=0x00;
}

/**
 * copies RESET ALL CONTROLLERS MIDI message
 * @param channel MIDI channel number 1-16 (0x00-0x0f).
 */
static INLINE void copy_reset_all_controllers(const U8 channel){
	MIDIsendBuffer[MIDIbytesToSend++]=(EV_CONTROLLER<<4)|channel;
	MIDIsendBuffer[MIDIbytesToSend++]=C_RESET_ALL;
	MIDIsendBuffer[MIDIbytesToSend++]=0x00;
}

/**
 * copies ALL NOTES OFF MIDI message
 * @param channel MIDI channel number 1-16 (0x00-0x0f).
 */
static INLINE void copy_all_notes_off(const U8 channel){
	MIDIsendBuffer[MIDIbytesToSend++]=(EV_CONTROLLER<<4)|channel;
	MIDIsendBuffer[MIDIbytesToSend++]=C_ALL_NOTES_OFF;
	MIDIsendBuffer[MIDIbytesToSend++]=0x00;
}

/**
 * copies OMNI OFF MIDI message
 * @param channel MIDI channel number 1-16 (0x00-0x0f).
 */
static INLINE void copy_omni_off(const U8 channel){
	MIDIsendBuffer[MIDIbytesToSend++]=(EV_CONTROLLER<<4)|channel;
	MIDIsendBuffer[MIDIbytesToSend++]=C_OMNI_OFF;
	MIDIsendBuffer[MIDIbytesToSend++]=0x00;
}

/**
 * copies OMNI ON MIDI message
 * @param channel MIDI channel number 1-16 (0x00-0x0f).
 */
static INLINE void copy_omni_on(const U8 channel){
	MIDIsendBuffer[MIDIbytesToSend++]=(EV_CONTROLLER<<4)|channel;
	MIDIsendBuffer[MIDIbytesToSend++]=C_OMNI_ON;
	MIDIsendBuffer[MIDIbytesToSend++]=0x00;
}

/**
 * copies MONO MIDI message
 * @param channel MIDI channel number 1-16 (0x00-0x0f).
 */
static INLINE void copy_mono(const U8 channel,const U8 numberOfMono){
	MIDIsendBuffer[MIDIbytesToSend++]=(EV_CONTROLLER<<4)|channel;
	MIDIsendBuffer[MIDIbytesToSend++]=C_MONO;
	MIDIsendBuffer[MIDIbytesToSend++]=numberOfMono;
}

/**
 * copies POLY MIDI message
 * @param channel MIDI channel number 1-16 (0x00-0x0f).
 */

static INLINE void copy_poly(const U8 channel, const U8 numberOfPoly){
	MIDIsendBuffer[MIDIbytesToSend++]=(EV_CONTROLLER<<4)|channel;
	MIDIsendBuffer[MIDIbytesToSend++]=C_POLY;
	MIDIsendBuffer[MIDIbytesToSend++]=numberOfPoly;
}

/** Silents midi channels (n to n-1) - useful when we have for example hanging notes.
*   @param numChannels - number of channel
*/

static INLINE void am_allNotesOff(U16 numChannels){
U16 iCounter;
  for(iCounter=0;iCounter<numChannels;++iCounter){
    all_notes_off(iCounter);
  }
}



#endif
