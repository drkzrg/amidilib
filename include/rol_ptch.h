#ifndef ROL_PTCH_H
#define ROL_PTCH_H

#include "c_vars.h"
#include "mdevtype.h"

#include "vendors.h"
#include "roland.h"

typedef enum ReverbMode{
  RV_HALL,
  RV_ROOM,
  RV_PLATE,
  RV_TAP_DELAY,
  RV_NONE
} eReverbMode;

#define NO_CHANGE 255

typedef struct midiModuleSettings{
    U8 vendorID;
    U8 deviceID;
    U8 modelID;
    U8 masterVolume;
    U8 masterBalance;  // only GM / GS

    /* MT-32 only */
    U8 reverbMode;
    U8 reverbTime;
    U8 reverbLevel;

} sMidiModuleSettings;

/** resets MT-32 module to factory settings with default timbre library */
void MT32Reset(void);

/** patch MT-32 module to GM timbres */
void patchMT32toGM(const BOOL bStandardGMDrumset);

/* these two functions turn parts (channels) 2-10 off on Roland CM-300 and CM-500 modules,
as well as many other Roland GS devices.

This is especially useful for those who route MIDI output through a Sound Canvas to a LA synth and need to silence the GS channels.
The allPartsOff() will also correct LA synth playback problems with some older Sierra games when played on the CM-500, in 'Mode A'.
allPartsOn() will revert all these settings. */

void  allPartsOn(void);
void  allPartsOff(void);

/* GS native */

// calculate Roland checksum for SysEX messages
/** Calculates checksum of Roland SysEx messages
*	@param buf_start pointer to the start of SysEx message buffer
*	@param buf_end pointer to an end of SysEx message buffer
*	@return calculated checksum */

U8 am_calcRolandChecksum(U8 *buf_start, U8 *buf_end);

void setupMidiDevice(const eMidiDeviceType device, const U8 channel);

#endif // ROL_PTCH_H
