#ifndef MACHINE_H
#define MACHINE_H

/**  Copyright 2007-2022 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#include "vartypes.h"

//_SND cookie bits
#define SND_CODEC (1 << 5)
#define SND_CON_MATRIX (1 << 4)
#define SND_DSP56001 (1 << 3)
#define SND_DMA_16BIT (1 << 2)
#define SND_DMA_8BIT (1 << 1)
#define SND_YM2149 (1 << 0)

// _FPU cookie bits
#define FPU060_internal (1 << 20)
#define FPU040_internal (1 << 19)

#define PSG_BIT 0
#define STEREO_PLAYBACK 1
#define DMA_REC_BIT 2
#define CODEC_BIT 3
#define DSP_BIT 4

typedef enum 
{
  UNKNOWN_MODEL = 0,
  ST = 1,
  MEGA_ST,
  ST_BOOK,
  STE,
  MSTE,
  TT,
  F030,
  F060,
  MILAN,
  MEDUSA,
  HADES,
  EAGLE,
  FIREBEE,
  EMU,
  MAX_ATARI_TYPE
} eAtariModel;

typedef enum 
{
  UNKNOWN_DISP = 0,
  SHIFTER_ST,
  SHIFTER_STE,
  SHIFTER_TT,
  VIDEL,
  SUPERVIDEL,
  ATIRADEON,
  MAX_VIDEO_HARDWARE_TYPE
} eVideoHardwareType;

typedef enum 
{
  MC68000 = 0,
  MC68010 = 10,
  MC68020 = 20,
  MC68030 = 30,
  MC68040 = 40,
  MC68060 = 60
} eCPU;

typedef enum 
{
  MT_MONO=0,
  MT_RGB,
  MT_VGA,
  MT_TV,
  MT_LCD,
  MT_DVI,
  MAX_MON_TYPE
} eMonitorType;

typedef struct _cookie 
{
  uint32 cookie;
  uint32 value;
} sCOOKIE;

typedef struct _osheader 
{
  uint16 os_entry;
  uint16 os_version;
  void *reseth;
  struct _osheader *os_beg;
  uint8 *os_end;
  uint8 *os_rsv1;
  uint8 *os_magic;
  int32 os_date;
  uint16 os_conf;
  uint16 os_dosdate;

  /* Available as of TOS 1.02 */
  uint8 **p_root;
  uint8 **p_kbshift;
  uint8 **p_run;
  uint8 *p_rsv2;
} sOSHEADER;

// custom structure for stroing current machine info

typedef struct MACHINEINFO 
{
  uint16 tos_version;                // high byte is the major revision number, and the low byte
                                     // is the minor revision number.
  uint16 os_config;                  // os_config>>1 (holds TOS version),
                                     // (os_config&0%0000000000000001) 0-NTSC, 1-PAL
  uint16 gemdos_ver;                 // GEMDOS version
  eAtariModel model;                 // enum from eModel
  eVideoHardwareType videoHardware;  // enum from eDisplay
  eMonitorType monType;              // monitor type
  uint32 stram;                      // available ST ram
  uint32 fastram;                    // available fast ram
  uint32 vram;                       // available video ram (SV/Radeon)
  int32 cpu;                        // value of _CPU cookie
  int32 mch;                        // value of _MCH cookie
  int32 vdo;                        // value of _VDO cookie
  int32 snd;                        // value of _SND cookie
  int32 fpu;                        // value of _FPU cookie
} sMachineInfo;

void checkMachine(void);
const char *getMachineName(const eAtariModel type);
const char *getVideoHardwareName(const eVideoHardwareType type);
const char *getMonitorName(const eMonitorType type);

int16 getCookie(const uint32 target, uint32 *p_value);
const sMachineInfo *getMachineInfo(void);
const sOSHEADER *getROMSysbase(void);

#endif
