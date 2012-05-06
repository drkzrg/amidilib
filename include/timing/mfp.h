
/**  Copyright 2007-2010 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#ifndef __MFP_H__
#define __MFP_H__

#ifndef PORTABLE
#include "c_vars.h"

#define MFP_STOP	0b00000000  /* Timer stop */
#define MFP_DIV4	0b00000001  /* div 4 */
#define MFP_DIV10	0b00000010  /* div 10 */
#define MFP_DIV16	0b00000011  /* div 16 */
#define MFP_DIV50	0b00000100  /* div 50 */
#define MFP_DIV64	0b00000101  /* div 64 */
#define MFP_DIV100	0b00000110  /* div 100 */
#define MFP_DIV200	0b00000111  /* div 200 */
#define MFP_EC		0b00001000  /* event count mode (TA/TB)  */ 
#define MFP_DEL4	0b00001001  /* delay 4 */
#define MFP_DEL10	0b00001010  /* delay 10 */
#define MFP_DEL16	0b00001011  /* delay 16 */
#define MFP_DEL50	0b00001100  /* delay 50 */
#define MFP_DEL64	0b00001101  /* delay 64 */
#define MFP_DEL100	0b00001110  /* delay 100 */
#define MFP_DEL200	0b00001111  /* delay 200 */

/** Utility function returns MFP mode and data settings for MFP.
*   @param freq - desired frequency
*   @param mode - pointer to unsigned long int for MFP mode of operation value
*   @param data - pointer to unsigned long int for MFP data value
*/
void getMFPTimerSettings(U32 freq,U8 *mode,U8 *data);

extern void installMIDIreplay(U8 mode,U8 data);
extern void deinstallMIDIreplay(void);

#endif

#endif