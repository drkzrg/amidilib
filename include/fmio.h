
/**  Copyright 2007-2010 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#ifndef __FMIO_H__
#define __FMIO_H__

#include <mintbind.h>
#include <mint/osbind.h>
#include "memory.h"
#include "c_vars.h"

/*XBIOS input/output devices*/
#define XB_DEV_AUX 0	/* Centronics interface */
#define XB_DEV_KBD 1    /* Keyboard port */
#define XB_DEV_MIDI 2	/* MIDI interface */

/* GEMDOS errors */
 #define GDOS_OK 0  /* No error */
 #define GDOS_ERR -1  /*Error*/
 #define GDOS_DRIVE_NOT_READY -2  /*Drive not ready*/
 #define GDOS_UNKNOWN_COMMAND -3  /*Unknown command*/
 #define GDOS_CRC_ERROR -4  /*CRC (checksum) error*/
 #define GDOS_BAD_REQ -5  /*Bad request*/
 #define GDOS_SEEK_ERROR -6  /*Seek error*/
 #define GDOS_UNKNOWN_MEDIA -7  /*Unknown media*/
 #define GDOS_SECTOR_NOT_FOUND -8  /*Sector not found*/
 #define GDOS_OUT_OF_PAPER -9  /*Out of paper*/
 #define GDOS_WRITE_FAULT -10  /*Write fault*/
 #define GDOS_READ_FAULT -11  /*Read fault*/
 #define GDOS_NVM_CHESUM_ERR -12  /*NVM checksum error*/
 #define GDOS_WRITE_PROTECTED_MEDIA -13  /*Write protected media*/
 #define GDOS_MEDIA_CHANGE_DETECTED -14  /*Media change detected*/
 #define GDOS_UNKNOWN_DEVICE -15  /*Unknown device*/
 #define GDOS_BAD_SECTOR_ON_FORMAT -16  /*Bad sectors on format*/
 #define GDOS_INSERT_OTHER_DISK_REQ -17  /*Insert other disk (request)*/

 #define GDOS_INVALID_GDOS_FUNC_NB -32  /*Invalid GEMDOS function number*/
 #define GDOS_FILE_NOT_FOUND -33  /*File not found*/
 #define GDOS_PATH_NOT_FOUND -34  /*Path not found*/
 #define GDOS_HANDLE_POOL_EXHAUSTED -35  /*Handle pool exhausted*/
 #define GDOS_ACCESS_DENIED -36  /*Access denied*/
 #define GDOS_INVALID_HANDLE -37  /*Invalid handle*/
 #define GDOS_INSUFFICIENT_MEMORY -39  /*Insufficient memory*/
 #define GDOS_INVALID_MEM_BLOCK_ADDR -40  /*Invalid memory block address*/
 #define GDOS_INVALID_DRIVE_SPEC -46  /*Invalid drive specification*/
 #define GDOS_NOT_THE_SAME_DRIVE -48  /*Not the same drive*/
 #define GDOS_NO_MORE_FILES -49  /*No more files*/
 #define GDOS_RECORD_LOCKED -58  /*Record is already locked*/
 #define GDOS_INVALID_LOCK_REMOVAL_REQ -59  /*Invalid lock removal request*/
 #define GDOS_ARG_RANGE_ERR -64  /*Argument range error*/
 #define GDOS_INTERNAL_ERR -65  /*GEMDOS internal error*/
 #define GDOS_INVALID_EXE_FMT -66  /*Invalid executable file format*/
 #define GDOS_MEM_BLOCK_GROWTH_FAIL -67  /*Memory block growth failure*/
 #define GDOS_TOO_MANY_SYMLINKS -80  /*Too many symbolic links*/
 #define GDOS_MOUNT_POINT_CROSSED -200  /*Mount point crossed*/

/**
 * loads file to specific type of memory(ST/TT RAM).
 *
 * @param szFileName NULL terminated full path with name
 * @param memFlag memory allocation preference flag
 * @param fileLenght file lenght in bytes
 * @return NULL - if error occured,
 * valid pointer - if file was loaded.
 */

void *loadFile(U8 *szFileName, eMemoryFlag memFlag, U32 *fileLenght);

/**
 * loads gets text description of actual GEMDOS error.
 *
 * @param iErr GEMDOS error code
 * @return pointer to const char * array with error description
 */

const U8 *getGemdosError(S16 iErr);

/**
 * gets text description of last GEMDOS error that occured.
 *
 * @return pointer to const char * array with error description
 */
const U8 *getLastGemdosError(void);

/**
 * turns supervisor mode ON.
 *
*/
static long  ssp = 0;

static inline void am_setSuperOn(void) {
  #ifdef DEBUG_BUILD
    amTrace((const U8*)"Entering supervisor\n");
  #endif
  ssp = Super(0);         /* enter supervisor mode */
}

/**
 * turns supervisor mode Off.
 *
*/
static inline void am_setSuperOff(void) {
    SuperToUser(ssp); /* return processor to user mode */
    
    #ifdef DEBUG_BUILD
      amTrace((const U8*)"Leaving supervisor\n");
    #endif
 }


#endif
