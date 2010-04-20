/**  Copyright 2008, 2009 Pawel Goralski
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

#include "INCLUDE\MIDI_CMD.H"
#include "INCLUDE\AMIDILIB.H"


/* small static buffer for sending MIDI commands */
static U8 g_midi_cmd_buffer[4];

/* common, channel voice messages */
void note_off (U8 channel, U8 note, U8 velocity )
{
	g_midi_cmd_buffer[0]=(EV_NOTE_OFF<<4)|(channel);
	g_midi_cmd_buffer[1]=note;
	g_midi_cmd_buffer[2]=velocity;
	MIDI_SEND_DATA(3,g_midi_cmd_buffer);
}

void note_on (U8 channel, U8 note, U8 velocity)
{
	g_midi_cmd_buffer[0]=(EV_NOTE_ON<<4)|channel;
	g_midi_cmd_buffer[1]=note;
	g_midi_cmd_buffer[2]=velocity;
	MIDI_SEND_DATA(3,g_midi_cmd_buffer);
}

/* or NOTE AFTERTOUCH */
void polyphonic_key_press(U8 channel, U8 note, U8 value)
{
	g_midi_cmd_buffer[0]=(EV_NOTE_AFTERTOUCH<<4)|channel;
	g_midi_cmd_buffer[1]=note;
	g_midi_cmd_buffer[2]=value;
	MIDI_SEND_DATA(3,g_midi_cmd_buffer);
}

void control_change(U8 controller, U8 channel, U8 value1, U8 value2)
{
	g_midi_cmd_buffer[0]=(EV_CONTROLLER<<4)|channel;
	g_midi_cmd_buffer[1]=controller;
	g_midi_cmd_buffer[2]=value1;
	g_midi_cmd_buffer[3]=value2;
}

void program_change(U8 channel, U8 programNb)
{
	g_midi_cmd_buffer[0]=(EV_PROGRAM_CHANGE<<4)|channel;
	g_midi_cmd_buffer[1]=programNb;
	MIDI_SEND_DATA(2,g_midi_cmd_buffer);
}

void channel_pressure (U8 channel, U8 value)
{
	g_midi_cmd_buffer[0]=(EV_CHANNEL_AFTERTOUCH<<4)|channel;
	g_midi_cmd_buffer[1]=value;
	MIDI_SEND_DATA(2,g_midi_cmd_buffer);
}

void pitch_bend (U8 channel, U16 value)
{
	g_midi_cmd_buffer[0]=(EV_PITCH_BEND<<4)|channel;
	g_midi_cmd_buffer[1]=(U8)((value>>4)&0x0F);
	g_midi_cmd_buffer[2]=(U8)(value&0x0F);
	MIDI_SEND_DATA(3,g_midi_cmd_buffer);
}

void pitch_bend_2 (U8 channel, U8 fbyte, U8 sbyte)
{
	g_midi_cmd_buffer[0]=(EV_PITCH_BEND<<4)|channel;
	g_midi_cmd_buffer[1]=fbyte;
	g_midi_cmd_buffer[2]=sbyte;
	MIDI_SEND_DATA(3,g_midi_cmd_buffer);

}

/* channel mode messages */
void all_sounds_off(U8 channel)
{
	g_midi_cmd_buffer[0]=(EV_CONTROLLER<<4)|channel;
	g_midi_cmd_buffer[1]=C_SOUNDS_OFF;
	g_midi_cmd_buffer[2]=0x00;

	MIDI_SEND_DATA(3,g_midi_cmd_buffer);

}

void reset_all_controllers(U8 channel)
{
	g_midi_cmd_buffer[0]=(EV_CONTROLLER<<4)|channel;
	g_midi_cmd_buffer[1]=C_RESET_ALL;
	g_midi_cmd_buffer[2]=0x00;

	MIDI_SEND_DATA(3,g_midi_cmd_buffer);
}

void all_notes_off(U8 channel)
{
	g_midi_cmd_buffer[0]=(EV_CONTROLLER<<4)|channel;
	g_midi_cmd_buffer[1]=C_ALL_NOTES_OFF;
	g_midi_cmd_buffer[2]=0x00;

	MIDI_SEND_DATA(3,g_midi_cmd_buffer);
}

void omni_off(U8 channel)
{
	g_midi_cmd_buffer[0]=(EV_CONTROLLER<<4)|channel;
	g_midi_cmd_buffer[1]=C_OMNI_OFF;
	g_midi_cmd_buffer[2]=0x00;
	MIDI_SEND_DATA(3,g_midi_cmd_buffer);
}

void omni_on(U8 channel)
{
	g_midi_cmd_buffer[0]=(EV_CONTROLLER<<4)|channel;
	g_midi_cmd_buffer[1]=C_OMNI_ON;
	g_midi_cmd_buffer[2]=0x00;
	MIDI_SEND_DATA(3,g_midi_cmd_buffer);
}

void mono(U8 channel, U8 numberOfMono)
{
	g_midi_cmd_buffer[0]=(EV_CONTROLLER<<4)|channel;
	g_midi_cmd_buffer[1]=C_MONO;
	g_midi_cmd_buffer[2]=numberOfMono;
	MIDI_SEND_DATA(3,g_midi_cmd_buffer);
}

void poly(U8 channel, U8 numberOfPoly)
{
	g_midi_cmd_buffer[0]=(EV_CONTROLLER<<4)|channel;
	g_midi_cmd_buffer[1]=C_POLY;
	g_midi_cmd_buffer[2]=numberOfPoly;
	MIDI_SEND_DATA(3,g_midi_cmd_buffer);
}
