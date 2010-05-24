
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

/* Roland CM-500 static tables */
#include	"include/c_vars.h"

/* SFX set, program number 57 */
extern const U8 *g_arCM500sfx[];


/* module default settings table */

const U8 *getCM500SFXName(U8 ubNoteNb)
{

 return(g_arCM500sfx[ubNoteNb]);
}


void  allPartsOn(void)
{
 /* TODO: */
}

void  allPartsOff(void)
{
 /* TODO: */
}

