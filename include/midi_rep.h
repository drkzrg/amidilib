#ifndef _MIDI_REP_H_
#define _MIDI_REP_H_

/**  Copyright 2007-2011 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#include "c_vars.h"
#include "amidiseq.h"

/** installs sequence replay routine (hooked to timer B atm) */
extern void installReplayRout(U8 mode,U8 data,VOIDFUNCPTR func);

/** deinstalls sequence replay routine (hooked to timer B atm) */
extern void deinstallReplayRout(void);

/** Checks if there is sequence currently playing. 
* @return TRUE is sequence is playing, FALSE otherwise.
*/
BOOL isSeqPlaying(void);

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
void muteTrack(const U16 trackNb,const BOOL bMute);

/** toggles between replay modes. 
 * "play once"(replay stops after end of tracks) and "play in loop" 
 * (replay restarts from beginning) */
void toggleReplayMode(void);

/** Returns currently active sequence.
 * @return pointer to currently active sequence
 */

void getCurrentSeq(sSequence_t **);

/** Outputs current sequence state. */
void printSequenceState();
void printMidiSendBufferState();

const U8 *getPlayModeStr(const ePlayMode mode);
const U8 *getPlayStateStr(const ePlayState state);

#ifdef IKBD_MIDI_SEND_DIRECT
void flushMidiSendBuffer();
#endif

#endif
