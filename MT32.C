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

/* Roland MT-32 static tables */
#include	"INCLUDE/C_VARS.H"

#ifdef __PUREC__
/*
switches off PURE C:
suspicious pointer conversion and unreachable code warnings
 */

#pragma warn -sus
#pragma warn -rch
#endif

/*instruments can be assigned to 1-8 part (2-8 channel, can be remmaped to channel 1-8)*/
extern U8 *g_arMT32instruments[];


/* mt-32 drumset (fixed channel 10) */
extern const U8 *g_arMT32rhythm[];


/* module default settings table */

const U8 *getMT32InstrName(U8 ubInstrNb)
{

 return(g_arMT32instruments[ubInstrNb]);
};

const U8 *getMT32RhythmName(U8 ubNoteNb)
{

 return(g_arMT32rhythm[ubNoteNb]);
};
