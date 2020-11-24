#ifndef _MIDI_REP_H_
#define _MIDI_REP_H_

/**  Copyright 2007-2020 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#include "c_vars.h"
#include "amidiseq.h"
#include "timing/mfp.h"

/** Checks if there is sequence currently playing. 
* @return TRUE is sequence is playing, FALSE otherwise.
*/
Bool isSeqPlaying(void);

/** stops sequence replay */
void stopSeq(void);

/** pause/unpause currently replayed sequence */
void pauseSeq(void);

/** play currently replayed sequence */
void playSeq(void);

/** setups sequence for replay. Sequence is stopped by default */
void initSeq(sSequence_t *seq, eTimerType timerType);

/** setups sequence for replay. Sequence is stopped by default */
/** for debug purposes, step not on timer */
void initSeqManual(sSequence_t *seq);

/** mutes selected track or first one if there is only one */
void muteTrack(const uint16 trackNb,const Bool bMute);

/** toggles between replay modes. 
 * "play once"(replay stops after end of tracks) and "play in loop" 
 * (replay restarts from beginning) */
void toggleReplayMode(void);

/** Returns currently active sequence (R / W).
 * @returns pointer to currently active sequence
 */
sSequence_t * const getActiveSequence();

/** destroys loaded Sequence.
*   @param pPtr pointer to a pointer with loaded sequence data. Passed pointer is null
*   if operation was completed succesfully. */

void amDestroySequence (sSequence_t **pPtr);

/** Outputs current sequence state. */
void printSequenceState(void);

const uint8 *getPlayStateStr(const uint16 mode);

#endif
