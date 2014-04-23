
/**  Copyright 2007-2014 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#ifndef ROLINSTR_H
#define ROLINSTR_H

#include "c_vars.h"

/* Roland modules header */
const U8 *getMT32InstrName(const U8 ubInstrNb);
const U8 *getMT32RhythmName(const U8 ubNoteNb);

/* MT-32, CM-32L, CM-64 */
const U8 *getCM32LInstrName(const U8 ubInstrNb);
const U8 *getCM32LRhythmName(const U8 ubNoteNb);

/* CM-500 */
const U8 *getCM500SFXName(const U8 ubNoteNb);

#endif // ROLINSTR_H
