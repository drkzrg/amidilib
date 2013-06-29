
/**  Copyright 2007-2010 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

/* Roland MT-32 static tables */
#include	"c_vars.h"

/*instruments can be assigned to 1-8 part (2-8 channel, can be remmaped to channel 1-8)*/
extern U8 *g_arMT32instruments[];


/* mt-32 drumset (fixed channel 10) */
extern const U8 *g_arMT32rhythm[];


/* module default settings table */

const U8 *getMT32InstrName(U8 ubInstrNb)
{

 return(g_arMT32instruments[ubInstrNb]);
}

const U8 *getMT32RhythmName(U8 ubNoteNb)
{

 return(g_arMT32rhythm[ubNoteNb]);
}
