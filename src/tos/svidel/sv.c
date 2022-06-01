
/**  Copyright 2007-2022 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#include "svidel/sv_defs.h"
#include "core/logger.h"
#include "core/amPrintf.h"

#define SV_VERSION      0x8001007CUL    /* SV version register */

// dumps SCREENINFO content to file/console
static inline void logScreenInfo(const RSCREENINFO *pPtr) 
{
  amTrace("\nSize of structure:\t0x%x"NL, pPtr->size);
  amTrace("Current screen mode:\t0x%x"NL, pPtr->devID); /* Device ID number */
  amTrace("Name:\t%s"NL, pPtr->name);        /* Friendly name of Screen    */
  amTrace("Flags:\t0x%x"NL, pPtr->scrFlags); /* Some flags                 */
  amTrace("Visible X res:\t0x%x (%d)"NL, pPtr->scrWidth,
        pPtr->scrWidth); /* Visible X res              */
  amTrace("Visible Y res:\t0x%x (%d)"NL, pPtr->scrHeight,
        pPtr->scrHeight); /* Visible Y res              */
  amTrace("Virtual X res:\t0x%x (%d)"NL, pPtr->virtWidth,
        pPtr->virtWidth); /* Virtual X res              */
  amTrace("Virtual Y res:\t0x%x (%d)"NL, pPtr->virtHeight,
        pPtr->virtHeight); /* Virtual Y res              */
  amTrace("color Planes:\t0x%x"NL, pPtr->scrPlanes); /* color Planes */
  amTrace("# of colors:\t0x%x (%d)"NL, pPtr->scrColors,
        pPtr->scrColors); /* # of colors                */
  amTrace("# of bytes to next line:\t0x%x"NL,
        pPtr->lineWrap); /* # of bytes to next line    */
  amTrace("# of bytes to next plane:\t0x%x"NL,
        pPtr->planeWarp); /* # of bytes to next plane   */
  amTrace("Screen format:\t0x%x"NL, pPtr->scrFormat); /* Screen format */
  amTrace("CLUT type:\t0x%x"NL, pPtr->scrClut); /* Type of clut               */
  amTrace("RED bits mask:\t0x%x"NL, pPtr->redBits);     /* Mask of Red Bits */
  amTrace("GREEN bits mask:\t0x%x"NL, pPtr->greenBits); /* Mask of Green Bits */
  amTrace("BLUE bits mask:\t0x%x"NL, pPtr->blueBits);   /* Mask of Blue Bits */
  amTrace("ALPHA bits mask:\t0x%x"NL, pPtr->alphaBits); /* Mask of Alpha Bits */
  amTrace("Bits organisation flags:\t0x%x"NL,
        pPtr->bitFlags); /* Bits organisation flags    */
  amTrace("Max. memory in this mode:\t0x%x"NL,
        pPtr->maxmem); /* Max. memory in this mode   */
  amTrace("Memory size of one frame buffer page:\t0x%x (%d bytes)"NL,
        pPtr->pagemem, pPtr->pagemem); /* Needed memory for one page */
  amTrace("Refresh rate:\t%d [Hz]"NL, pPtr->refresh); /* refresh rate in Hz */
  amTrace("========================================="NL,0);
}


// needs supervisor
const uint32 getSVfwVersion(void)
{
  uint32 ver=0;

#if defined(TARGET_CT60)
   uint32 *addr=(uint32 *)SV_VERSION;
    ver=*addr;
#endif

    return ver;
}
