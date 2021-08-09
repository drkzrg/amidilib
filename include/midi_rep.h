#ifndef _MIDI_REP_H_
#define _MIDI_REP_H_

/**  Copyright 2007-2021 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#include "vartypes.h"
#include "amidiseq.h"
#include "timing/mfp.h"

/** Checks if there is sequence currently playing. 
* @return TRUE is sequence is playing, FALSE otherwise.
*/
Bool isAmSequencePlaying(void);

/** stops sequence replay */
void stopAmSequence(void);

/** pause/unpause currently replayed sequence */
void pauseAmSequence(void);

/** play currently replayed sequence */
void playAmSequence(void);

/** setups sequence for replay. Sequence is stopped by default */
void initAmSequence(sSequence_t *seq, eTimerType timerType);

/** setups sequence for replay. Sequence is stopped by default */
/** for debug purposes, step not on timer */
void initAmSequenceManual(sSequence_t *seq);

/** mutes selected track or first one if there is only one */
void muteAmTrack(const uint16 trackNb,const Bool bMute);

/** toggles between replay modes. 
 * "play once"(replay stops after end of tracks) and "play in loop" 
 * (replay restarts from beginning) */
void toggleAmReplayMode(void);

/** Returns currently active sequence (R / W).
 * @returns pointer to currently active sequence
 */
sSequence_t * const getActiveAmSequence();

/** destroys loaded Sequence.
*   @param pPtr pointer to a pointer with loaded sequence data. Passed pointer is null
*   if operation was completed succesfully. */

void destroyAmSequence (sSequence_t **pPtr);

/** Outputs current sequence state. */
void printAmSequenceState(void);

const uint8 *getPlayStateStr(const uint16 mode);

#endif
