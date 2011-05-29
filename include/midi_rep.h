#ifndef _MIDI_REP_H_
#define _MIDI_REP_H_

#include "c_vars.h"
#include "amidiseq.h"

#ifdef PORTABLE
void installReplayRout(U8 mode,U8 data);
void deinstallReplayRout(void);
#else
// install and deinstall routines
extern void installReplayRout(U8 mode,U8 data);
extern void deinstallReplayRout(void);
#endif

// update sequence function called from exception routine
void sequenceUpdate(void);

//sequence control interface
BOOL isSeqPlaying(void);
void stopSeq(void);
void pauseSeq(void);
void playSeq(void);
void initSeq(sSequence_t *seq);
void muteTrack(U16 trackNb,BOOL bMute);

//toggles between play once and play in loop modes.
void toggleReplayMode(void);

#endif