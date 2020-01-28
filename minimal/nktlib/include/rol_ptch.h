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
    uint8 vendorID;
    uint8 deviceID;
    uint8 modelID;
    uint8 masterVolume;
    uint8 masterBalance;  // only GM / GS

    /* MT-32 only */
    uint8 reverbMode;
    uint8 reverbTime;
    uint8 reverbLevel;

} sMidiModuleSettings;

/** resets MT-32 module to factory settings with default timbre library */
void MT32Reset(void);

/** patch MT-32 module to GM timbres */
void patchMT32toGM(const bool bStandardGMDrumset);

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

uint8 am_calcRolandChecksum(uint8 *buf_start, uint8 *buf_end);

// sets custom text message on mt-32 lcd screen
void setMT32Message(const uint8 *msg);

void setupMidiDevice(const eMidiDeviceType device, const uint8 channel);

#endif // ROL_PTCH_H
