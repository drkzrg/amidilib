#ifndef SV_DEFS_H
#define SV_DEFS_H

/**  Copyright 2007-2022 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#include "ct60.h"

#ifndef C_SupV
#define C_SupV 0x53757056L /* SuperVidel 'SupV' cookie id */
#endif

/* SuperVidel registers */
#define SV_MEM_FLAG 0xA0000000UL
#define SV_VRAMBASE 0xA1000000UL
#define SV_VERSION 0x8001007CUL /* SV version register R/W */

/* SuperVidel SuperBlitter registers */

/*
  FIFO port: Write to this 32bit port to queue SuperBlitter register settings.
  Starting a blitter operation requires nine register writes, starting at
  0x80010058 and going up to and including 0x80010078, in that
  order. When the FIFO holds at least nine longwords and the blitter
  is idle, nine longwords will be read out from the FIFO to overwrite
  the registers at 0x80010058-0x80010078, and the blitter will be
  started, if the control register holds such settings.

  FIFO empty flag: Is '1' when the blitter FIFO holds less than 9 longwords.
  FIFO full flag: Is '1' when the blitter FIFO holds 500 or more longwords.
  This register is available from FW revision 9.
*/

#define SV_FIFO_PORT 0x80010080UL
#define SV_FIFO_FULL (1 << 0)
#define SV_FIFO_EMPTY (1 << 1)

#define SV_SB_SRCADDR1 0x80010058UL
#define SV_SB_SRCADDR2 0x8001005CUL
#define SV_SB_DESTADDR 0x80010060UL

/* The amount of bytes that are to be copied in a horizontal line, minus 1.
 * To copy 1 byte, write 0 here.*/
#define SV_SB_LINECOUNT 0x80010064UL
#define SV_SB_LINECOUNT_MASK (~(0xFFFFF800UL))       /* first ten bits */
#define SV_SB_LINECOUNT_ALPHA_MASK (~(0x00FFFFFFUL)) /* last 8 bits */

/* The amount of bytes that are to be added to the line start adress
after a line has been copied, in source1, in order to reach the next
line start address. Negative amounts (address decrements) can be
realised by using numbers starting with '1' here (in bit 26).*/

#define SV_SB_SRC1_LOFFSET 0x80010068UL
#define SV_SB_SRC2_LOFFSET 0x8001006CUL
#define SV_SB_DEST_LOFFSET 0x80010070UL

/*
Src1 adress count mask:
  A mask to apply to the source1 address counter while incrementing
  it during the write phase. The value set will make the adress wrap
  at that value. Only 2^n values are possible: 0,1,2,4,8...128. Note
  that we're counting 64-bit words here, not bytes, since the blitter
  works with 64-bit words internally. Also note that it is only the
  counter, that is added to the src1 base address, that is affected by
  this mask, not the src1 base address itself. Setting the mask to 0
  will turn off masking of the address counter (but it can only count to
  255 anyway).
Src2 address count mask:
  The same as the src1 address mask above, but for src2.
Number of lines:
  The amount of horizontal lines to do.
*/

#define SV_SB_ADDR_MASK 0x80010074UL
#define SV_SB_ADDR_LINE_COUNT_MASK (~(0x00000FFFUL))
#define SV_SB_ADDR_SRC2_ADDR_MOD_MASK (~(0x00FF0000UL))
#define SV_SB_ADDR_SRC1_ADDR_MOD_MASK (~(0xFF000000UL))

/*

Start/Busy:
When the Busy bit reads as '0' the Start bit may be set to '1' to start
the SuperBlitter again. But make sure that the other SuperBlitter
registers are properly set up first! While this bit is set to '1' the
SuperBlitter is busy performing earlier copy operations.
Blit mode (Yes, the bits are not enumerated in the expected order..) :

Code Blit mode:
0b0000 dest = src1
0b0010 Reserved
0b0100 Reserved
0b0110 Reserved
0b1000 Reserved
0b1010 Reserved
0b1100 Reserved
0b1110 Reserved

0b0001 dest = src1 if src2 != 0, else dest = dest
Each byte of src1 is written to dest if the corresponding byte of src2 is
not 0. For 8-bit chunky mode: src1=src2=sprite data
For 16-bit sprites: src1=sprite data, src2=mask data

0b0011 dest = src1 * src1.alpha + src2 * (256-src1.alpha)
32-bit pixel alpha blending. Each pixel is R,G,B,A. 8 bits per channel.
Only the alpha channel of src1 is used.

0b0101 dest = src1 if src1.bit5 == 1, else dest = dest
Each 16bit word of src1 is written to dest if its bit 5 is 1. This is bit 0 of
the green channel in a 16bit mode.

0b0111 dest = src1 * src1.alpha * alpha_reg +
src2 * (256 - src1.alpha * alpha_reg )
32-bit pixel alpha blending with global alpha effect too. Each pixel is
R,G,B,A. 8 bits per channel. The global alpha value comes from the
register at 0x80010064.

0b1001 Reserved
0b1011 Reserved
0b1101 Reserved
0b1111 Reserved

INT enable & Done INT flag
---------------------------
Writing INT enable to 1 makes the blitter generate an interrupt
request when it completes a blitting job, and Done INT flag will be
set to 1. You must clear it in your handler by writing 0 to Done INT
flag. Make sure you put your ISR pointer in vector number 0xC6
(address 0x318) before starting a blitter job with interrupts enabled.
Since rev 6 firmware the Done INT flag can be written to '1' to make
the blitter immediately generate a Done interrupt, if it was idle (if
the blitter was busy with an operation nothing will happen).
INT_enable must also be set for this to work.
Replace Enable and Replace Red, Green, Blue

This enables replacing of the RGB part of source1 in 32bit RGBA
blitter modes (0b0011 and 0b0111) with the replacement register
values Replace Red, Green, Blue.

*/

#define SV_SB_STATUS_CONTROL 0x80010078UL

/////////////////////////////////

#define BPint8C (0x0007)
#define BPint32 (0x0005)

#ifndef SVEXT_BASERES
#define SVEXT_BASERES (a)((a & 0xf) << 9)
#endif

#define SVEXT_FORMAT (0x0040) /* 0 = 4:3, 1 = wide resolution  */
#define SVEXT_DUAL (0x8000)   /* enable dual screen, don't touch atm */

#ifndef SVEXT
#define SVEXT (0x4000)
#endif

// for VideoMode enumeration

// Vsetscreen modecode extended flags
#define HORFLAG 0x200         /* double width */
#define HORFLAG2 0x400        /* width increased */
#define VESA_600 0x800        /* SVGA 600 lines */
#define VESA_768 0x1000       /* SVGA 768 lines */
#define VERTFLAG2 0x2000      /* double height */
#define DEVID 0x4000          /* bits 11-3 used for devID */
#define VIRTUAL_SCREEN 0x8000 /* width * 2 and height * 2, 2048 x 2048 max */

// OVERSCAN and PAL flags are used for select refresh frequency:
// OVERSCAN | PAL | Freq
// ---------+-----+-----
//        0 |  0  | 56
//        0 |  1  | 60
//        1 |  0  | 70
//        1 |  1  | 85
//

#define GET_DEVID(x) (((x)&DEVID) ? ((x & 0x3FF8) >> 3) : -1)
#define SET_DEVID(x) ((((x) << 3) & 0x3FF8) | DEVID)

#define MI_MAGIC (('M' << 8) | 'I')
#define NV_MAGIC (('N' << 8) | 'V')
#define SV_MAGIC(device) (('S' << 8) | device)

// Vsetscreen function enumeration
enum {
  R_CMD_GETMODE = 0,
  R_CMD_SETMODE,
  R_CMD_GETINFO,
  R_CMD_ALLOCPAGE,
  R_CMD_FREEPAGE,
  R_CMD_FLIPPAGE,
  R_CMD_ALLOCMEM,
  R_CMD_FREEMEM,
  R_CMD_SETADR,
  R_CMD_ENUMMODES,
  R_CMD_TESTMODE,
  R_CMD_COPYPAGE,
  R_CMD_FILLMEM,
  R_CMD_COPYMEM,
  R_CMD_TEXTUREMEM,
  R_CMD_GETVERSION
};

#define ENUMMODE_CONT 1
#define ENUMMODE_EXIT 0

typedef struct rscreeninfo {
  uint32 size;        /* Size of structure          */
  uint32 devID;       /* modecode                   */
  uint32 name[64];    /* Friendly name of Screen    */
  uint32 scrFlags;    /* some Flags                 */
  uint32 frameadr;    /* Address of framebuffer     */
  uint32 scrHeight;   /* visible X res              */
  uint32 scrWidth;    /* visible Y res              */
  uint32 virtHeight;  /* virtual X res              */
  uint32 virtWidth;   /* virtual Y res              */
  uint32 scrPlanes;   /* color Planes               */
  uint32 scrColors;   /* # of colors                */
  uint32 lineWrap;    /* # of Bytes to next line    */
  uint32 planeWarp;   /* # of Bytes to next plane   */
  uint32 scrFormat;   /* screen Format              */
  uint32 scrClut;     /* type of clut               */
  uint32 redBits;     /* Mask of Red Bits           */
  uint32 greenBits;   /* Mask of Green Bits         */
  uint32 blueBits;    /* Mask of Blue Bits          */
  uint32 alphaBits;   /* Mask of Alpha Bits         */
  uint32 genlockBits; /* Mask of Genlock Bits       */
  uint32 unusedBits;  /* Mask of unused Bits        */
  uint32 bitFlags;    /* Bits organisation flags    */
  uint32 maxmem;      /* max. memory in this mode   */
  uint32 pagemem;     /* needed memory for one page */
  uint32 max_x;       /* max. possible width        */
  uint32 max_y;       /* max. possible heigth       */
  uint32 refresh;     /* refresh rate in Hz */
  uint32 pixclock;    /* pixelclock */
} RSCREENINFO;

typedef struct {
  uint16 number; /* Video mode number */
  uint16 width;  /* Size */
  uint16 height;
  uint16 depth;   /* bits per plane */
  uint16 refresh; /* refresh rate, applicable only for Radeon */
  uint16 flags;
} xbiosmode_t;

#define F030_PAL 0xFFFF9800UL /* 1024 bytes palette register */

const uint32 getSvFwVersion(void);

#endif
