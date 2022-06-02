
/**  Copyright 2011-21 Pawel Goralski
    This file is part of Nidhoggr engine.
    See license.txt for licensing information.
*/

#include <mint/cookie.h>
#include <mint/falcon.h>

#include "core/amprintf.h"
#include "core/logger.h"
#include "core/machine.h"
#include "memory/memory.h"
#include "ct60.h"
#include "sv_defs.h"

static const char *sAtariModels[MAX_ATARI_TYPE] =
{
    "Unknown",       "Atari ST",         "Atari Mega ST",
    "Atari ST Book", "Atari STe",        "Atari Mega STe",
    "Atari TT",      "Atari Falcon 030", "Atari Falcon CT60",
    "Milan",         "Medusa",           "Hades",
    "Eagle",         "Firebee",          "Emulator"
};

static const char *sAtariVideoHardware[MAX_VIDEO_HARDWARE_TYPE] =
{
  "Unknown", "Shifter ST", "Shifter STe", "Shifter TT",
  "Videl",   "SuperVidel", "ATI Radeon"
};

static const char *sMonType[MAX_MON_TYPE] =
{
  "MONO", "RGB", "VGA",
  "TV",   "LCD", "DVI"
};

static sMachineInfo machineInfo;

const sMachineInfo *getMachineInfo(void) 
{ 
  return &machineInfo; 
}

const char *getMachineName(const eAtariModel type) 
{
  return sAtariModels[type];
}

const char *getVideoHardwareName(const eVideoHardwareType type) 
{
  return sAtariVideoHardware[type];
}

const char *getMonitorName(const eMonitorType type) 
{
  return sMonType[type];
}

void checkMachine(void) 
{
  uint32 mch, vdo, snd, cpu, fpu;
  const sOSHEADER *pOS = getROMSysbase();

  amTrace("Check machine"NL,0);

  // TOS version, GEMDOS/AES version
  machineInfo.tos_version = pOS->os_version;
  machineInfo.os_config = pOS->os_conf;
  machineInfo.gemdos_ver = Sversion();

  // check cpu, fpu, machine type etc..
  // got cookie jar?
  if (getCookie(C__CPU, &cpu) == C_NOTFOUND) 
  {
    machineInfo.cpu = 0;
    machineInfo.model = ST;
    machineInfo.videoHardware = SHIFTER_ST;
    machineInfo.snd = 1;  // assume only PSG
    machineInfo.fpu = -1L;

    return;
  }

  if (getCookie(C__CPU, &cpu) == C_FOUND) 
  {
    machineInfo.cpu = cpu;
    amTrace("CPU cookie found: 0x%x"NL,cpu);

    switch (cpu) 
    {
      case MC68000: 
      {
        amTrace("CPU: mc68000"NL,0);
      } break;
      case MC68010:
      {
        amTrace("CPU: mc68010"NL,0);
      } break;
      case MC68020:
      {
        amTrace("CPU: mc68020"NL,0);
      } break;
      case MC68030:
      {
        amTrace("CPU: mc68030"NL,0);
      } break;
      case MC68040:
      {
        amTrace("CPU: mc68040"NL,0);
      } break;
      case MC68060:
      {
        amTrace("CPU: mc68060"NL,0);
      } break;
      default: 
      { 
        amTrace("Error: Unknown CPU!"NL,0);
      };
    };

  }
  else
  {
    amTrace("Error: No _CPU information found!"NL,0);
  }

  if (getCookie(C__MCH, &mch) == C_FOUND) 
  {
    // check machine type
    const uint16 major = (uint16)((mch & 0xFFFF0000UL) >> 16);
    const uint16 minor = (uint16)(mch & 0x0000FFFFUL);

    machineInfo.mch = mch;

    switch (major)
    {
      case 0:
      {
        // ST
        machineInfo.model = ST;
        amTrace("Machine: ST"NL,0);
      } break;
      case 1:
      {
        // STe or ST book or mega
        switch (minor)
        {
          case 0:
          {
            machineInfo.model = STE;
            amTrace("Machine: STe"NL,0);
          } break;
          case 8:
          {
            machineInfo.model = ST_BOOK;
            amTrace("Machine: ST Book"NL,0);
          } break;
          case 16:
          {
            machineInfo.model = MSTE;
            amTrace("Machine: Mega STe"NL,0);

          } break;
        }
      } break;
      case 2:
      {
        // TT
        machineInfo.model = TT;
        amTrace("Machine: TT"NL,0);
      } break;
      case 3:
      {
        // Falcon
        machineInfo.model = F030;
        amTrace("Machine: F030"NL,0);

      } break;
    }
  }
  else
  {
    amTrace("Error: No _MCH information!"NL,0);

    machineInfo.mch = -1L;
    machineInfo.model = 0;
  }

  if (getCookie(C__VDO, &vdo) == C_FOUND) 
  {
    machineInfo.vdo = vdo;
    // check shifter type
    const uint16 major = ((mch & 0xFFFF0000L) >> 16);

    switch (major)
    {
      case 0:
      {
        machineInfo.videoHardware = SHIFTER_ST;
        amTrace("Video: Shifter ST"NL,0);
      } break;
      case 1: {
        machineInfo.videoHardware = SHIFTER_STE;
        amTrace("Video: Shifter STe"NL,0);
      } break;
      case 2:
      {
        machineInfo.videoHardware = SHIFTER_TT;
        amTrace("Video: Shifter TT"NL,0);
      } break;
      case 3:
      {
        machineInfo.videoHardware = VIDEL;
        amTrace("Video: Videl"NL,0);
      } break;
      default:
      {
        machineInfo.videoHardware = UNKNOWN_DISP;
        amTrace("Video: Unknown"NL,0);
      } break;
    };
  }
  else
  {
    machineInfo.vdo = -1L;
    amTrace("Error: No _VDO information!"NL,0);
  }

  // get monitor type
  switch (VgetMonitor()) 
  {
    case STmono:
    {
      machineInfo.monType = MT_MONO;
      amTrace("Monitor output: mono"NL,0);
    } break;
    case STcolor:
    {
      machineInfo.monType = MT_RGB;
      amTrace("Monitor output: RGB"NL,0);
    } break;
    case VGAcolor:
    {
      machineInfo.monType = MT_VGA;
      amTrace("Monitor output: VGA"NL,0);
    } break;
    case TVcolor:
    {
      machineInfo.monType = MT_TV;
      amTrace("Monitor output: TV"NL,0);
    } break;
    case 4:
    {
      machineInfo.monType = MT_LCD;
      amTrace("Monitor output: LCD"NL,0);
    } break;
    case 5:
    {
      machineInfo.monType = MT_DVI;
      amTrace("Monitor output: DVI"NL,0);
    } break;
  };

  // TODO: detect DVI/dual dvi SV

  if (getCookie(C__SND, &snd) == C_FOUND)
  {
    machineInfo.snd = snd;
    amTrace("Sound hardware found:"NL,0);

    if ((snd & SND_CODEC) == SND_CODEC)
    {
      amTrace("Codec \t",0);
    }

    if ((snd & SND_CON_MATRIX) == SND_CON_MATRIX)
    {
      amTrace("Connection Matrix \t",0);
    }

    if ((snd & SND_DSP56001) == SND_DSP56001)
    {
      amTrace("DSP56001 \t",0);
    }

    if ((snd & SND_DMA_16BIT) == SND_DMA_16BIT)
    {
      amTrace("16-bit DMA Sound \t",0);
    }

    if ((snd & SND_DMA_8BIT) == SND_DMA_8BIT)
    {
      amTrace("8-bit DMA Sound \t",0);
    }

    if ((snd & SND_YM2149) == SND_YM2149)
    {
      amTrace("ym2149 \t",0);
    }

    amTrace(""NL,0);
  }
  else
  {
    amTrace("Error: _SND information not found!"NL,0);
    machineInfo.snd = -1L;
  }

  if (getCookie(C__FPU, &fpu) == C_FOUND)
  {
    machineInfo.fpu = fpu;
    amTrace("FPU found. "NL,0);  // TODO
  }
  else
  {
    amTrace("_FPU information not found!"NL,0);
    machineInfo.fpu = -1L;
  }

  // check only if falcon hardware was detected
  if (machineInfo.model == F030)
  {
    // accelerators, PCI bridges etc, graphics cards
    uint32 ct60 = 0, pci = 0, sv = 0;

    if (getCookie(C_CT60, &ct60) == C_FOUND)
    {
      amTrace("CT60 detected."NL,0);
      machineInfo.model = F060;

      if (getCookie(C__PCI, &pci) == C_FOUND)
      {
        // check graphics card availability
        if ((unsigned long)Physbase() >= 0x01000000UL)
        {
          machineInfo.videoHardware = ATIRADEON;
          machineInfo.vram = ct60_vmalloc(0, -1);
          amTrace("CTPCI / ATI Radeon detected."NL,0);
        }
      }  // check pci bios
      else if (getCookie(C_SupV, &sv) == C_FOUND)
      {
        machineInfo.videoHardware = SUPERVIDEL;
        amTrace("Supervidel detected. Firmware ver: 0x%x"NL,0, getSVfwVersion());
        machineInfo.vram = ct60_vmalloc(0, -1);
      }  // check Supervidel
    }    // check ct60
  }

  // check available ST/TT-RAM
  machineInfo.stram = amGetFreeMemory(MF_STRAM);
  machineInfo.fastram = amGetFreeMemory(MF_FASTRAM);

  if (machineInfo.videoHardware == SUPERVIDEL ||
      machineInfo.videoHardware == ATIRADEON)
  {
    amTrace("RAM: ST-RAM:%d, Fast-RAM:%d, Video-RAM: %d"NL,machineInfo.stram, machineInfo.fastram, machineInfo.vram);
  }
  else
  {
    amTrace("RAM: ST-RAM:%d, Fast-RAM:%d"NL,machineInfo.stram, machineInfo.fastram);
  }

  return;
}

int16 getCookie(const uint32 target, uint32 *p_value)
{
  sCOOKIE *cookie_ptr = (sCOOKIE *)0L;

  if (*((uint32 *)0x5A0UL) == 0L)
  {
    return C_NOTFOUND;  // no cookie jar present, assume plain STf
  }

  cookie_ptr = *(sCOOKIE **)0x5A0;

  if (cookie_ptr != NULL) {
    do {
      if (cookie_ptr->cookie == target) {
        if (p_value != NULL) *p_value = cookie_ptr->value;
        return C_FOUND;
      }

    } while ((cookie_ptr++)->cookie != 0L);
  }

  return C_NOTFOUND;
}

const sOSHEADER *getROMSysbase(void) 
{
  sOSHEADER *osret;
  osret = (*(sOSHEADER **)0x4F2)->os_beg;
  return osret;
}
