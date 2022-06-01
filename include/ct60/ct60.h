#ifndef CT60_H
#define CT60_H

/**  Copyright 2011-21 Pawel Goralski
    This file is part of Nidhoggr engine.
    See license.txt for licensing information.
*/

#include <mint/osbind.h>
#include "vartypes.h"

/* CT60LIB helper functions */
/* CT60 XBIOS function bindings and enums */

/** CPU temperature constants */
typedef enum {
  CT60_CELCIUS = 0,  /** temperature returned in Celsius */
  CT60_FARENHEIT = 1 /** temperature returned in Fahrenheit */
} eCT60tempFormat;

/** Error handling constants */

#ifndef ERROR
#define ERROR -1
#endif

#ifndef E_OK
#define E_OK 0
#endif

#ifndef EBADRQ
#define EBADRQ -5
#endif

/** Read / Write parameter from EPROM */
typedef enum {
  CT60_MODE_READ = 0, /** The value has to be read.*/
  CT60_MODE_WRITE = 1 /** The value has to be written */
} eRWparameter;

/** Valid types of parameters that can be read/written to EPROM */
typedef enum {
  CT60_PARAM_TOSRAM = 0L,
  CT60_BLITTER_SPEED = 1L,
  CT60_CACHE_DELAY = 2L,
  CT60_BOOT_ORDER = 3L,
  CT60_FPU = 4L,
  CT60_PARAM_OFFSET_TLV = 10L
} eTypeOfParameter;

/** Possible values for parameters */
/** CT60_PARAM_TOSRAM=0L */
#define TOSINFLASH 0L /** TOS in flash rom */
#define TOSINSDRAM 1L /** TOS in SDRAM */

/** CT60_BLITTER_SPEED=1L */
#define BLITTER_NORMAL 0L /** Blitter 8/10 mhz */
#define BLITTER_FAST 1L   /** Blitter 16/20 mhz */

/** CT60_CACHE_DELAY=2L */
#define DELAY_CACHE_BIT 0x01L
#define COPYBACK_BIT 0x02L

/**  CT60_BOOT_ORDER=3L */

typedef enum {
  NEW_BOOT_0 = 0L,
  NEW_BOOT_1 = 1L,
  NEW_BOOT_2 = 2L,
  NEW_BOOT_3 = 3L,
  OLD_BOOT_0 = 4L,
  OLD_BOOT_1 = 5L,
  OLD_BOOT_2 = 6L,
  OLD_BOOT_3 = 7L
} eBootOrder;

/**  CT60_FPU=4L */
#define FPU_BIT 0x01L /** if bit 0 cleared, FPU disabled */

/**  CT60_PARAM_OFFSET_TLV=10L */
/** Value contains signed offset in points. Point == 2.8 �C.*/

/** Error while reading/writing of the parameters to EPROM */

typedef enum {
  RW_EBADRQ = -5,  /** if the parameter is >=15 */
  RW_EWRITF = -10, /** for a write fault */
  RW_EUNDEV = -15  /** if the flash device is not found */
} eRWerrorType;

/** CT60 cache control constants */
typedef enum {
  CACHE_ENABLE = 1,   /** enable the caches */
  CACHE_DISABLE = 0,  /** disable the caches */
  CACHE_NOCHANGE = -1 /** no change */
} eCT60CacheControl;

/** Generic Cache Control constants */

/** CacheCtrl() opcodes */
typedef enum {
  CACHE_OP_CHECK =
      0, /**  return 0 to check that function is implemented, Param=0 */
  CACHE_OP_FLUSH_DATA = 1,  /** Flush Data Cache, Param=0 */
  CACHE_OP_FLUSH_INSTR = 2, /** Flush Instruction Cache, Param=0 */
  CACHE_OP_FLUSH_DATA_INSTR =
      3,                     /** Flush Data and Instruction Cache, Param=0 */
  CACHE_OP_INQUIRE_DATA = 4, /** Inquire data cache mode, Param=0 */
  CACHE_OP_SET_DATA_MODE =
      5, /** Set data cache mode, invalidate/flush caches, eCacheCtrlMode as
            Param */
  CACHE_OP_INQUIRE_INSTR = 6, /**  Inquire instruction cache mode, Param=0 */
  CACHE_OP_SET_INSTR_MODE = 7 /** Set instruction cache mode, invalidate/flush
                                 caches, eCacheCtrlMode as Param */
} eCacheCtrlOpcode;

/** \remarks Note: Under the CT60 XBIOS, OpCodes 5/7 have the same effect :<br>
  mode : 	0 = disable all caches.<br>
                1 = enable all caches.<br>
                 All caches are :<br>
                 Data Cache, Store Buffer, Branch Cache, Instruction Cache.<br>
*/

typedef enum {
  CACHECTRL_ENABLE = 1, /** enable cache */
  CACHECTRL_DISABLE = 0 /** disable cache */
} eCacheCtrlMode;

/***************** CT60 function declarations */
/** CT60  XBIOS function 0xc60a
 @param tempFormat Accepts constants specified in eCT60tempFormat.It tells the
function in which units the temperature has to be returned, in Celsius or the
Fahrenheit.<br>
* \return Returns ERROR (-1) if there is read error.
*/
extern int32 ct60_ReadCoreTemperature(const eCT60tempFormat tempFormat);

/** CT60  XBIOS function 0xc60b<br>
* Read write parameter inside the flash eprom.
* @param mode It specifies if value has to be written or read. The possible
* parameters are in eTypeOfParameter
* @param type_param as specified in eTypeOfParameter
* @param value Value to be read / written.
* \return Returns inquired value or error: EBADRQ (-5) if parameter is >=15,
* WWRITF (-10) for a write fault, EUNDEV (-15) if the flash device is not found.
* All error codes are specified in enum eRWerrorType.
*/
extern int32 ct60_RWparameter(const eRWparameter mode, const eTypeOfParameter type_param,
                            const int32 value);

/** CT60 XBIOS function 0xc60c<br>
* @param cache_mode accepts constants from eCT60CacheControl
* \return Returns the contents of the CACR */
extern uint32 ct60_cache(const eCT60CacheControl cache_mode);

/** CT60 XBIOS function 0xc60d<br>
Flushes CT60 caches.
*/
extern int32 ct60_FlushCache(void);

/** CacheCtrl() XBIOS function 160<br>
* @param opcode as specified in eCacheCtrlOpcode
* @param Param needed for opcode CACHE_OP_INQUIRE_INSTR and
* CACHE_OP_SET_INSTR_MODE. The correct values are specified in
* eCacheCtrlMode(CACHECTRL_ENABLE, CACHECTRL_DISABLE).
* \remarks Compatible with MilanTOS
* \return Returns Inquired value, E_OK (0) or BADRQ (-5) if the OpCode is > 7
*/
extern int32 ct60_CacheCtrl(const eCacheCtrlOpcode opcode, const int16 Param);

// ct60 video ram malloc
#define ct60_vmalloc(mode, value) \
  (long)trap_14_wwl((short int)0xc60e, (short int)(mode), (long)(value))

#endif
