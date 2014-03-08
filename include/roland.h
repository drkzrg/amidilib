
/**  Copyright 2007-2010 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#ifndef __ROLAND_H__
#define __ROLAND_H__

/* settings for cm500 GS source */
#define GS_MODEL_ID 0x16
#define GS_DEVICE_ID 0x10

/* settings for cm500 LA source */
#define LA_MODEL_ID 0x42
#define LA_DEVICE_ID 0x10

#define DEV_ID1 	0x00
#define DEV_ID2		0x01
#define DEV_ID3 	0x02
#define DEV_ID4		0x03
#define DEV_ID5 	0x04
#define DEV_ID6		0x05
#define DEV_ID7 	0x06
#define DEV_ID8		0x07
#define DEV_ID9 	0x08
#define DEV_ID10	0x09
#define DEV_ID11 	0x0A
#define DEV_ID12	0x0b
#define DEV_ID13 	0x0c
#define DEV_ID14	0x0d
#define DEV_ID15 	0x0e
#define DEV_ID16	0x0f

/* TODO:
 * functions for maintaining current device state, initial settings etc..   */
/* blocking/unblocking functions on certain devices
   (e.g. Roland MT-32 recognizes the standard MIDI controllers for
   Volume (controller 7), Panpot (controller 10), Sustain (controller 64), and Modulation (controller 1))  */

/* Roland modules header */
const U8 *getMT32InstrName(const U8 ubInstrNb);
const U8 *getMT32RhythmName(const U8 ubNoteNb);

/* MT-32, CM-32L, CM-64 */
const U8 *getCM32LInstrName(const U8 ubInstrNb);
const U8 *getCM32LRhythmName(const U8 ubNoteNb);

/** resets MT-32 module to factory settings with default timbre library */
void MT32Reset(void);

/* CM-500 */
const U8 *getCM500SFXName(const U8 ubNoteNb);

/* these two functions turn parts (channels) 2-10 off on Roland CM-300 and CM-500 modules,
as well as many other Roland GS devices.

This is especially useful for those who route MIDI output through a Sound Canvas to a LA synth and need to silence the GS channels.
The allPartsOff() will also correct LA synth playback problems with some older Sierra games when played on the CM-500, in 'Mode A'.
allPartsOn() will revert all these settings.

*/

void  allPartsOn(void);
void  allPartsOff(void);

/* GS native */

// calculate Roland checksum for SysEX messages
/** Calculates checksum of Roland SysEx messages
*	@param buf_start pointer to the start of SysEx message buffer
*	@param buf_end pointer to an end of SysEx message buffer
*	@return calculated checksum */
U8 am_calcRolandChecksum(U8 *buf_start, U8 *buf_end);


#endif
