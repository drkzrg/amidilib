
/**  Copyright 2007-2019 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#ifndef ROLINSTR_H
#define ROLINSTR_H

#include "c_vars.h"

/* Roland modules header */
const uint8 *getMT32InstrName(const uint8 ubInstrNb);
const uint8 *getMT32RhythmName(const uint8 ubNoteNb);

/* MT-32, CM-32L, CM-64 */
const uint8 *getCM32LInstrName(const uint8 ubInstrNb);
const uint8 *getCM32LRhythmName(const uint8 ubNoteNb);

/* CM-500 */
const uint8 *getCM500SFXName(const uint8 ubNoteNb);

#endif // ROLINSTR_H
