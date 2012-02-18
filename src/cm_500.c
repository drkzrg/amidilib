
/**  Copyright 2007-2010 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

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

