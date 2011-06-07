#ifndef _MIDI_REP_H_
#define _MIDI_REP_H_

/**  Copyright 2007-2011 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#include "c_vars.h"
#include "amidiseq.h"

// main AMIDILIB sequence replay interface
#ifdef PORTABLE
/** installs sequence replay routine (portable version) */
void installReplayRout(U8 mode,U8 data);

/** deinstalls sequence replay routine (portable version) */
void deinstallReplayRout(void);
#else
/** installs sequence replay routine (hooked to timer B atm) */
extern void installReplayRout(U8 mode,U8 data);

/** deinstalls sequence replay routine (hooked to timer B atm) */
extern void deinstallReplayRout(void);
#endif

/** update sequence function called from interrupt routine */
void sequenceUpdate(void);

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
void initSeq(sSequence_t *seq);

/** mutes selected track or first one if there is only one */
void muteTrack(U16 trackNb,BOOL bMute);

/** toggles between replay modes. 
 * "play once"(replay stops after end of tracks) and "play in loop" 
 * (replay restarts from beginning) */
void toggleReplayMode(void);

/** Returns currently active sequence. 
 * @return pointer to currently active sequence 
 */
 
const sSequence_t *getCurrentSeq(void);


#endif