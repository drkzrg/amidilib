
/**  Copyright 2007-2010 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/


#ifndef __MIDI_CMD_H__
#define __MIDI_CMD_H__

#include "c_vars.h"

/* common, channel voice messages */

/**
 * sends NOTE OFF MIDI message (key depressed)
 *
 * @param channel MIDI channel number 1-16 (0x00-0x0f).
 * @param note specifies which note to play 0-127 (0x00-0x7F)
 * @param velocity with what velocity 0-127 (0x00-0x7F)
 */
void note_off (U8 channel, U8 note, U8 velocity );
/**
 * sends NOTE ON MIDI message (key pressed)
 *
 * @param channel MIDI channel number 1-16 (0x00-0x0f).
 * @param note specifies which note to play 0-127 (0x00-0x7F)
 * @param velocity with what velocity 0-127 (0x00-0x7F)
 */
void note_on (U8 channel, U8 note, U8 velocity);

/**
 * sends POLYPHONIC KEY PRESSURE MIDI message (note aftertouch)
 *
 * @param channel MIDI channel number 1-16 (0x00-0x0f).
 * @param note specifies which note to play 0-127 (0x00-0x7F)
 * @param value value 0-127 (0x00-0x7F)
 */
void polyphonic_key_press(U8 channel, U8 note, U8 value);

/**
 * sends CONTROL CHANGE MIDI message
 * @param controller specifies which controller to use, full list is in EVENTS.H (from midi specification)
 * and additional for XMIDI are in XMIDI.H. Value can be 0-127 (0x00-0x7F)
 * @param channel MIDI channel number 1-16 (0x00-0x0f).
 * @param value value sent to given controller 0-127 (0x00-0x7F)
 */
void control_change(U8 controller, U8 channel, U8 value1,U8 value2);
/**
 * sends PROGRAM CHANGE MIDI message
 * @param channel MIDI channel number 1-16 (0x00-0x0f).
 * @param programNb program number 1-128 (0x00-0x7F)
 */
void program_change(U8 channel, U8 programNb);
/**
 * sends CHANNEL PRESSURE MIDI message
 * @param channel MIDI channel number 1-16 (0x00-0x0f).
 * @param value program number 0-127 (0x00-0x7F)
 * @remarks valid only for GS sound source
 */
void channel_pressure (U8 channel, U8 value);
/**
 * sends PITCH BEND MIDI message
 * @param channel MIDI channel number 1-16 (0x00-0x0f).
 * @param value signed U16 value -8192 - 0 - +8191 (0x0000 - 0x4000 - 0x7F7F)
 */
void pitch_bend (U8 channel, U16 value );

/**
 * sends PITCH BEND MIDI message
 * @param channel MIDI channel number 1-16 (0x00-0x0f).
 * @param fbyte U8 value which gives with sbyte S16 value with range: -8192 - 0 - +8191 (0x0000 - 0x4000 - 0x7F7F)
 * @param sbyte U8 value which gives with fbyte S16 value with range: -8192 - 0 - +8191 (0x0000 - 0x4000 - 0x7F7F)
 */
void pitch_bend_2 (U8 channel, U8 fbyte, U8 sbyte);

/* channel mode messages */
/**
 * sends ALL SOUNDS OFF MIDI message
 * @param channel MIDI channel number 1-16 (0x00-0x0f).
 */
void all_sounds_off(U8 channel);	/* for GS sources only */
/**
 * sends RESET ALL CONTROLLERS MIDI message
 * @param channel MIDI channel number 1-16 (0x00-0x0f).
 */
void reset_all_controllers(U8 channel);
/**
 * sends ALL NOTES OFF MIDI message
 * @param channel MIDI channel number 1-16 (0x00-0x0f).
 */
void all_notes_off(U8 channel);
/**
 * sends OMNI OFF MIDI message
 * @param channel MIDI channel number 1-16 (0x00-0x0f).
 */
void omni_off(U8 channel);

/**
 * sends OMNI ON MIDI message
 * @param channel MIDI channel number 1-16 (0x00-0x0f).
 */
void omni_on(U8 channel);

/**
 * sends MONO MIDI message
 * @param channel MIDI channel number 1-16 (0x00-0x0f).
 */
void mono(U8 channel, U8 numberOfMono);  /*?*/

/**
 * sends POLY MIDI message
 * @param channel MIDI channel number 1-16 (0x00-0x0f).
 */
void poly(U8 channel, U8 numberOfPoly);  /*?*/


/* SysEX */


#endif
