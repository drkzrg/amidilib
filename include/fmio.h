
/**  Copyright 2007-2021 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#ifndef __FMIO_H__
#define __FMIO_H__

#include "c_vars.h"
#include "memory/memory.h"

/*XBIOS input/output devices*/
 #define XB_DEV_AUX 0		/* Centronics interface */
 #define XB_DEV_KBD 1    	/* Keyboard port */
 #define XB_DEV_MIDI 2		/* MIDI interface */

typedef enum EGDOSERROR
{
	/* GEMDOS errors */
	 GDOS_OK = 0,  							/* OK, No error */
	 GDOS_ERR = -1,  						/*Error*/
	 GDOS_DRIVE_NOT_READY = -2,  			/*Drive not ready*/
	 GDOS_UNKNOWN_COMMAND = -3,  			/*Unknown command*/
	 GDOS_CRC_ERROR = -4,  					/*CRC (checksum) error*/
	 GDOS_BAD_REQ = -5,  					/*Bad request*/
	 GDOS_SEEK_ERROR = -6,  				/*Seek error*/
	 GDOS_UNKNOWN_MEDIA = -7,  				/*Unknown media*/
	 GDOS_SECTOR_NOT_FOUND = -8,  			/*Sector not found*/
	 GDOS_OUT_OF_PAPER = -9,  				/*Out of paper*/
	 GDOS_WRITE_FAULT = -10,  				/*Write fault*/
	 GDOS_READ_FAULT = -11,  				/*Read fault*/
	 GDOS_NVM_CHESUM_ERR = -12,  			/*NVM checksum error*/
	 GDOS_WRITE_PROTECTED_MEDIA = -13,  	/*Write protected media*/
	 GDOS_MEDIA_CHANGE_DETECTED = -14,  	/*Media change detected*/
	 GDOS_UNKNOWN_DEVICE = -15,  			/*Unknown device*/
	 GDOS_BAD_SECTOR_ON_FORMAT = -16,  		/*Bad sectors on format*/
	 GDOS_INSERT_OTHER_DISK_REQ = -17,  	/*Insert other disk (request)*/
	 GDOS_INVALID_GDOS_FUNC_NB = -32,  		/*Invalid GEMDOS function number*/
	 GDOS_FILE_NOT_FOUND = -33,  			/*File not found*/
	 GDOS_PATH_NOT_FOUND = -34,  			/*Path not found*/
	 GDOS_HANDLE_POOL_EXHAUSTED = -35,  	/*Handle pool exhausted*/
	 GDOS_ACCESS_DENIED = -36,  			/*Access denied*/
	 GDOS_INVALID_HANDLE = -37,  			/*Invalid handle*/
	 GDOS_INSUFFICIENT_MEMORY = -39,  		/*Insufficient memory*/
	 GDOS_INVALID_MEM_BLOCK_ADDR = -40, 	/*Invalid memory block address*/
	 GDOS_INVALID_DRIVE_SPEC = -46,  		/*Invalid drive specification*/
	 GDOS_NOT_THE_SAME_DRIVE = -48,  		/*Not the same drive*/
	 GDOS_NO_MORE_FILES = -49,  			/*No more files*/
	 GDOS_RECORD_LOCKED = -58,  			/*Record is already locked*/
	 GDOS_INVALID_LOCK_REMOVAL_REQ = -59,  	/*Invalid lock removal request*/
	 GDOS_ARG_RANGE_ERR = -64,  			/*Argument range error*/
	 GDOS_INTERNAL_ERR = -65,  				/*GEMDOS internal error*/
	 GDOS_INVALID_EXE_FMT = -66,  			/*Invalid executable file format*/
	 GDOS_MEM_BLOCK_GROWTH_FAIL = -67,  	/*Memory block growth failure*/
	 GDOS_TOO_MANY_SYMLINKS = -80,  		/*Too many symbolic links*/
	 GDOS_MOUNT_POINT_CROSSED = -200,  		/*Mount point crossed*/
 } eGdosError;
 
 const uint8 *getGemdosError(const eGdosError iErrcode);

/**
 * gets text description of last GEMDOS error that occured.
 *
 * @return pointer to const char * array with error description
 */
const uint8 *getLastGemdosError(void);

/**
 * loads file to specific type of memory(ST/TT RAM).
 * 
 * @param szFileName NULL terminated full path with name
 * @param memFlag memory allocation preference flag
 * @param fileLenght file lenght in bytes
 * @return NULL - if error occured,
 * valid pointer - if file was loaded.
 */

void *loadFile(const uint8 *szFileName, const eMemoryFlag memFlag, uint32 *fileLenght);

/**
 * loads gets text description of actual GEMDOS error.
 *
 * @param iErr GEMDOS error code
 * @return pointer to const char * array with error description
 */

int32 saveFile(const uint8 *szFileName, const void *memBlock, const uint32 memBlockSize);


#endif
