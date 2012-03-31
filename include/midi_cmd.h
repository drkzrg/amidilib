
/**  Copyright 2007-2010 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/


#ifndef __MIDI_CMD_H__
#define __MIDI_CMD_H__

#include "c_vars.h"
#include "midi_send.h"
#include "events.h"

/* small static buffer for sending MIDI commands */
static U8 g_midi_cmd_buffer[4];

/* common, channel voice messages */
/**
 * sends NOTE OFF MIDI message (key depressed)
 *
 * @param channel MIDI channel number 1-16 (0x00-0x0f).
 * @param note specifies which note to play 0-127 (0x00-0x7F)
 * @param velocity with what velocity 0-127 (0x00-0x7F)
 */

static INLINE void note_off (U8 channel, U8 note, U8 velocity )
{
	g_midi_cmd_buffer[0]=(EV_NOTE_OFF<<4)|(channel);
	g_midi_cmd_buffer[1]=note;
	g_midi_cmd_buffer[2]=velocity;
	MIDI_SEND_DATA(3,g_midi_cmd_buffer);
}

/**
 * sends NOTE ON MIDI message (key pressed)
 *
 * @param channel MIDI channel number 1-16 (0x00-0x0f).
 * @param note specifies which note to play 0-127 (0x00-0x7F)
 * @param velocity with what velocity 0-127 (0x00-0x7F)
 */
static INLINE void note_on (U8 channel, U8 note, U8 velocity)
{
	g_midi_cmd_buffer[0]=(EV_NOTE_ON<<4)|channel;
	g_midi_cmd_buffer[1]=note;
	g_midi_cmd_buffer[2]=velocity;
	MIDI_SEND_DATA(3,g_midi_cmd_buffer);
}

/**
 * sends POLYPHONIC KEY PRESSURE MIDI message (note aftertouch)
 *
 * @param channel MIDI channel number 1-16 (0x00-0x0f).
 * @param note specifies which note to play 0-127 (0x00-0x7F)
 * @param value value 0-127 (0x00-0x7F)
 */

static INLINE void polyphonic_key_press(U8 channel, U8 note, U8 value)
{
	g_midi_cmd_buffer[0]=(EV_NOTE_AFTERTOUCH<<4)|channel;
	g_midi_cmd_buffer[1]=note;
	g_midi_cmd_buffer[2]=value;
	MIDI_SEND_DATA(3,g_midi_cmd_buffer);
}
/**
 * sends CONTROL CHANGE MIDI message
 * @param controller specifies which controller to use, full list is in EVENTS.H (from midi specification)
 * and additional for XMIDI are in XMIDI.H. Value can be 0-127 (0x00-0x7F)
 * @param channel MIDI channel number 1-16 (0x00-0x0f).
 * @param value value sent to given controller 0-127 (0x00-0x7F)
 */

static INLINE void control_change(U8 controller, U8 channel, U8 value1, U8 value2)
{
	g_midi_cmd_buffer[0]=(EV_CONTROLLER<<4)|channel;
	g_midi_cmd_buffer[1]=controller;
	g_midi_cmd_buffer[2]=value1;
	g_midi_cmd_buffer[3]=value2;
}

/**
 * sends PROGRAM CHANGE MIDI message
 * @param channel MIDI channel number 1-16 (0x00-0x0f).
 * @param programNb program number 1-128 (0x00-0x7F)
 */

static INLINE void program_change(U8 channel, U8 programNb)
{
	g_midi_cmd_buffer[0]=(EV_PROGRAM_CHANGE<<4)|channel;
	g_midi_cmd_buffer[1]=programNb;
	MIDI_SEND_DATA(2,g_midi_cmd_buffer);
}

/**
 * sends CHANNEL PRESSURE MIDI message
 * @param channel MIDI channel number 1-16 (0x00-0x0f).
 * @param value program number 0-127 (0x00-0x7F)
 * @remarks valid only for GS sound source
 */

static INLINE void channel_pressure (U8 channel, U8 value)
{
	g_midi_cmd_buffer[0]=(EV_CHANNEL_AFTERTOUCH<<4)|channel;
	g_midi_cmd_buffer[1]=value;
	MIDI_SEND_DATA(2,g_midi_cmd_buffer);
}

/**
 * sends PITCH BEND MIDI message
 * @param channel MIDI channel number 1-16 (0x00-0x0f).
 * @param value signed U16 value -8192 - 0 - +8191 (0x0000 - 0x4000 - 0x7F7F)
 */
static INLINE void pitch_bend (U8 channel, U16 value)
{
	g_midi_cmd_buffer[0]=(EV_PITCH_BEND<<4)|channel;
	g_midi_cmd_buffer[1]=(U8)((value>>4)&0x0F);
	g_midi_cmd_buffer[2]=(U8)(value&0x0F);
	MIDI_SEND_DATA(3,g_midi_cmd_buffer);
}

/**
 * sends PITCH BEND MIDI message
 * @param channel MIDI channel number 1-16 (0x00-0x0f).
 * @param fbyte U8 value which gives with sbyte S16 value with range: -8192 - 0 - +8191 (0x0000 - 0x4000 - 0x7F7F)
 * @param sbyte U8 value which gives with fbyte S16 value with range: -8192 - 0 - +8191 (0x0000 - 0x4000 - 0x7F7F)
 */

static INLINE void pitch_bend_2 (U8 channel, U8 fbyte, U8 sbyte)
{
	g_midi_cmd_buffer[0]=(EV_PITCH_BEND<<4)|channel;
	g_midi_cmd_buffer[1]=fbyte;
	g_midi_cmd_buffer[2]=sbyte;
	MIDI_SEND_DATA(3,g_midi_cmd_buffer);
}

/* channel mode messages */
/**
 * sends ALL SOUNDS OFF MIDI message
 * @param channel MIDI channel number 1-16 (0x00-0x0f).
 */

static INLINE void all_sounds_off(U8 channel)
{
	g_midi_cmd_buffer[0]=(EV_CONTROLLER<<4)|channel;
	g_midi_cmd_buffer[1]=C_SOUNDS_OFF;
	g_midi_cmd_buffer[2]=0x00;

	MIDI_SEND_DATA(3,g_midi_cmd_buffer);

}

/**
 * sends RESET ALL CONTROLLERS MIDI message
 * @param channel MIDI channel number 1-16 (0x00-0x0f).
 */
static INLINE void reset_all_controllers(U8 channel)
{
	g_midi_cmd_buffer[0]=(EV_CONTROLLER<<4)|channel;
	g_midi_cmd_buffer[1]=C_RESET_ALL;
	g_midi_cmd_buffer[2]=0x00;

	MIDI_SEND_DATA(3,g_midi_cmd_buffer);
}

/**
 * sends ALL NOTES OFF MIDI message
 * @param channel MIDI channel number 1-16 (0x00-0x0f).
 */
static INLINE void all_notes_off(U8 channel)
{
	g_midi_cmd_buffer[0]=(EV_CONTROLLER<<4)|channel;
	g_midi_cmd_buffer[1]=C_ALL_NOTES_OFF;
	g_midi_cmd_buffer[2]=0x00;

	MIDI_SEND_DATA(3,g_midi_cmd_buffer);
}

/**
 * sends OMNI OFF MIDI message
 * @param channel MIDI channel number 1-16 (0x00-0x0f).
 */
static INLINE void omni_off(U8 channel)
{
	g_midi_cmd_buffer[0]=(EV_CONTROLLER<<4)|channel;
	g_midi_cmd_buffer[1]=C_OMNI_OFF;
	g_midi_cmd_buffer[2]=0x00;
	MIDI_SEND_DATA(3,g_midi_cmd_buffer);
}

/**
 * sends OMNI ON MIDI message
 * @param channel MIDI channel number 1-16 (0x00-0x0f).
 */
static INLINE void omni_on(U8 channel)
{
	g_midi_cmd_buffer[0]=(EV_CONTROLLER<<4)|channel;
	g_midi_cmd_buffer[1]=C_OMNI_ON;
	g_midi_cmd_buffer[2]=0x00;
	MIDI_SEND_DATA(3,g_midi_cmd_buffer);
}

/**
 * sends MONO MIDI message
 * @param channel MIDI channel number 1-16 (0x00-0x0f).
 */
static INLINE void mono(U8 channel, U8 numberOfMono)
{
	g_midi_cmd_buffer[0]=(EV_CONTROLLER<<4)|channel;
	g_midi_cmd_buffer[1]=C_MONO;
	g_midi_cmd_buffer[2]=numberOfMono;
	MIDI_SEND_DATA(3,g_midi_cmd_buffer);
}

/**
 * sends POLY MIDI message
 * @param channel MIDI channel number 1-16 (0x00-0x0f).
 */

static INLINE void poly(U8 channel, U8 numberOfPoly){
	g_midi_cmd_buffer[0]=(EV_CONTROLLER<<4)|channel;
	g_midi_cmd_buffer[1]=C_POLY;
	g_midi_cmd_buffer[2]=numberOfPoly;
	MIDI_SEND_DATA(3,g_midi_cmd_buffer);
}

#endif
