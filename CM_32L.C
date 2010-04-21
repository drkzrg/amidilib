
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

/* Roland CM-32L, CM-64 static tables */
#include	"INCLUDE/C_VARS.H"
#include	"INCLUDE/AMIDILIB.H"

#ifdef __PUREC__
/*
switches off PURE C:
suspicious pointer conversion and unreachable code warnings
 */

#pragma warn -sus
#pragma warn -rch
#endif

/*instruments can be assigned to 1-8 part (channel 2-9, can be remmaped to channel 1-8)*/
extern const U8 *g_arCM32Linstruments[];


/* drumset like in MT-32, but with additional sfx *only* for part 10 (channel 11)*/
/* the mapping is key note -> sound, program number 128 */
extern const U8 *g_arCM32Lrhythm[];


/* MT32 reset to factory defaults SysEx message */
static U8 g_arReset[]={0xf0,0x41,0x10,0x16,0x12,0x7f,0x01,0xf7};

/* MT32toGM bank patches */
/*give an include ?*/

static U8 g_arMT32toGM[]={
0x00

};

static U8 g_arMT32emptyBank[]={
0x00

};


/* module default settings table */

const U8 *getCM32LInstrName(U8 ubInstrNb)
{

 return(g_arCM32Linstruments[ubInstrNb]);
};

const U8 *getCM32LRhythmName(U8 ubNoteNb)
{

 return(g_arCM32Lrhythm[ubNoteNb]);
};

void MT32Reset(void)
{
	MIDI_SEND_DATA(8,g_arReset);
}

void patchMT32toGM(void)
{

}


