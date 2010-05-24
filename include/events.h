/*  Copyright 2008, 2009 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.

    AMIDILIB is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    AMIDILIB is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with AMIDILIB.  If not, see <http://www.gnu.org/licenses/>.*/

#ifndef __EVENTS_H__
#define __EVENTS_H__

/* Standard MIDI events definitions */

/* MIDI Channel Events  */
#define EV_NOTE_OFF             0x08       /* Note Off			    0x8		note number		    velocity */
#define EV_NOTE_ON              0x09       /* Note On			    0x9		note number		    velocity */
#define EV_NOTE_AFTERTOUCH      0x0A       /* Note Aftertouch	    0xA		note number		    aftertouch value */
#define EV_CONTROLLER           0x0B       /* Controller			    0xB		controller number	controller value */
#define EV_PROGRAM_CHANGE       0x0C       /* Program Change		    0xC		program number		not used */
#define EV_CHANNEL_AFTERTOUCH   0x0D       /* Channel Aftertouch		0xD		aftertouch value	not used */
#define EV_PITCH_BEND           0x0E       /* Pitch Bend		        0xE		pitch value (LSB)	pitch value (MSB) */


/* CONTROLLER (EV_CONTROLLER) EVENTS */

#define C_BANK_SELECT   0x00        /* Bank Select */
#define C_MODULATION    0x01	    /* Modulation */
#define C_BREATH        0x02        /* Breath Controller */
#define C_FOOT          0x04        /* Foot Controller */
#define C_PORTAMENTO_TIME   0x05    /* Portamento Time */
#define C_DATA_ENTRY    0x06        /* Data Entry (MSB) */
#define C_MAIN_VOL      0x07        /* Main Volume */
#define C_BALANCE       0x08        /* Balance */
#define C_PAN           0x0A        /* Pan */
#define C_EXPRESSION    0x0B        /* Expression Controller */
#define C_EFFECT1       0x0C		/* Effect Control 1 */
#define C_EFFECT2       0x0D        /* Effect Control 2 */
#define C_GP1           0x10        /* General-Purpose Controller 1 */
#define C_GP2           0x11        /* General-Purpose Controller 2 */
#define C_GP3           0x12        /* General-Purpose Controller 3 */
#define C_GP4           0x13		/* General-Purpose Controller 4 */
 /*	LSB's for controller 0-31 */
#define C_LSB0          0x20
#define C_LSB1          0x21
#define C_LSB2          0x22
#define C_LSB3          0x23
#define C_LSB4          0x24
#define C_LSB5          0x25
#define C_LSB6          0x26
#define C_LSB7          0x27
#define C_LSB8          0x28
#define C_LSB9          0x29
#define C_LSB10         0x2A
#define C_LSB11         0x2B
#define C_LSB12         0x2C
#define C_LSB13         0x2D
#define C_LSB14         0x2E
#define C_LSB15         0x2F
#define C_LSB16         0x30
#define C_LSB17         0x31
#define C_LSB18         0x32
#define C_LSB19         0x33
#define C_LSB20         0x34
#define C_LSB21         0x35
#define C_LSB22         0x36
#define C_LSB23         0x37
#define C_LSB24         0x38
#define C_LSB25         0x39
#define C_LSB26         0x3A
#define C_LSB27         0x3B
#define C_LSB28         0x3C
#define C_LSB29         0x3D
#define C_LSB30         0x3E
#define C_LSB31         0x3F

#define C_DAMPER_PEDAL  0x40    /* Damper pedal (sustain) */
#define C_PORTAMENTO    0x41	/* Portamento */
#define C_SOSTENUTO     0x42    /* Sostenuto */
#define C_SOFT_PEDAL    0x43    /* Soft Pedal */
#define C_LEGATO_FSW    0x44    /* Legato Footswitch */
#define C_HOLD2         0x45    /* Hold 2 */
#define C_SND_CTRL1     0x46    /* Sound Controller 1 (default: Timber Variation) */
#define C_SND_CTRL2     0x47	/* Sound Controller 2 (default: Timber/Harmonic Content) */
#define C_SND_CTRL3     0x48    /* Sound Controller 3 (default: Release Time) */
#define C_SND_CTRL4     0x49    /* Sound Controller 4 (default: Attack Time) */

#define C_SND_CTRL5     0x04A   /* Sound Controllers 5-10 */
#define C_SND_CTRL6     0x04B
#define C_SND_CTRL7     0x04C
#define C_SND_CTRL8     0x04D
#define C_SND_CTRL9     0x04E
#define C_SND_CTRL10    0x04F
#define C_GP_CTRL5      0x50    /* General Purpose controllers 5-8 */
#define C_GP_CTRL6      0x51
#define C_GP_CTRL7      0x52
#define C_GP_CTRL8      0x53
#define C_PORTAMENTO_CONTROL 0x54    /* Portamento Control */
#define C_EFFECTS1_DEPTH    0x5B    /* Effects 1 Depth (formerly External Effects Depth) */
#define C_EFFECTS2_DEPTH    0x5C    /* Effects 2 Depth (formerly Tremolo Depth) */
#define C_EFFECTS3_DEPTH    0x5D    /* Effects 3 Depth (formerly Chorus Depth) */
#define C_EFFECTS4_DEPTH    0x5E    /* Effects 4 Depth (formerly Celeste Detune) */
#define C_EFFECTS5_DEPTH    0x5F    /* Effects 5 Depth (formerly Phaser Depth)  */
#define C_DATA_INC          0x60    /* Data Increment */
#define C_DATA_DEC          0x61    /* Data Decrement */
#define C_NRPNB_LSB         0x62    /* Non-Registered Parameter Number (LSB) */
#define C_NRPNB_MSB         0x63    /* Non-Registered Parameter Number (MSB) */
#define C_RPN_LSB           0x64    /* Registered Parameter Number (LSB) */
#define C_RPN_MSB           0x65    /* Registered Parameter Number (MSB) */
#define C_SOUNDS_OFF		0x78    /* All Sounds off*/
#define C_RESET_ALL			0x79	/* Reset all controllers */
#define C_MODE_MSG1         0x79    /* Mode Messages 1-7*/
#define C_MODE_MSG2         0x7A
#define C_ALL_NOTES_OFF		0x7B
#define C_MODE_MSG3         0x7B
#define C_OMNI_OFF			0x7C
#define C_MODE_MSG4         0x7C
#define C_OMNI_ON	        0x7D
#define C_MODE_MSG5         0x7D
#define C_MONO				0x7E
#define C_MODE_MSG6         0x7E
#define C_POLY				0x7F
#define C_MODE_MSG7         0x7F


/***** META EVENTS */
#define EV_META 0xFF

/* META EVENT (EV_META) TYPES */
#define MT_SEQ_NB       0x00            /* defines the pattern number of a Type 2 MIDI file or the number of a sequence in a Type 0 or Type 1 MIDI file */
#define MT_TEXT         0x01            /* text string in ASCII, not NULL terminated! */
#define MT_COPYRIGHT    0x02            /* copyright info, not NULL terminated */
#define MT_SEQNAME      0x03            /* sequence/track name ASCII, not NULL terminated */
#define MT_INSTRNAME    0x04            /* instrument name ASCII, not NULL terminated */
#define MT_LYRICS       0x05            /* lyrics */
#define MT_MARKER       0x06            /* marker */
#define MT_CUEPOINT     0x07            /* cue point */
#define MT_PROGRAM_NAME 0x08            /* program name */
#define MT_DEVICE_NAME  0x09            /* device (port) name */
#define MT_CH_PREFIX    0xFF            /* midi channel prefix */
#define MT_MIDI_CH      0x20            /* midi channel, obsolete! */
#define MT_MIDI_PORT    0x21            /* midi port, obsolete!*/
#define MT_EOT          0x2F            /* end of midi track */
#define MT_SET_TEMPO    0x51
#define MT_SMPTE_OFFSET 0x54
#define MT_TIME_SIG     0x58            /* time signature */
#define MT_KEY_SIG      0x59            /* key signature */
#define MT_SEQ_SPEC     0x7F            /* sequencer specific data */

/***** SySEX Events */
#define EV_SOX      0xF0 				/* or SOX - Start of Exclusive */

/* System common messages, for all channels in system */
#define SC_MTCQF        0xF1            /* Midi time code quarter frame, 1 byte */
#define SC_SONG_POS_PTR 0xF2            /* Song position pointer, 2 data bytes */
#define SC_SONG_SELECT  0xF3            /* Song select 0-127, 1 data byte*/
#define SC_UNDEF1       0xF4            /* undefined */
#define SC_UNDEF2       0xF5            /* undefined */
#define SC_TUNE_REQUEST 0xF6            /* tune request, no data bytes */
#define EV_EOX		0xF7 				/* or EOX - End of Exclusive */

/***** PROGRAM CHANGE EVENTS */
/* System real time messages */
#define SR_TC       0xF8    /* timing clock */
#define SR_UNDEF1   0xF9    /* undefined */
#define SR_START    0xFA    /* START */
#define SR_CONTINUE 0xFB    /* continue */
#define SR_STOP     0xFC    /* stop */
#define SR_UNDEF2   0xFD    /* undefined */
#define SR_ASENSE	0xFE 	/* transmit at about 250 milliseconds interval */
#define SR_RESET    0xFF    /* system reset */

#endif
