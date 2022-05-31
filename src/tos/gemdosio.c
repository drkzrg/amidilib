
/**  Copyright 2007-2021 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#include <mint/ostruct.h>
#include <mint/osbind.h>
#include "memory/memory.h"
#include "gemdosio.h"

#include "core/amprintf.h"

static const char *g_arGEMDOSerror[71]= { 
  "No error.",\
  "Error.",\
  "Drive not ready.",\
  "Unknown command.",\
  "CRC (checksum) error.",\
  "Bad request.",\
  "Seek error.",\
  "Unknown media.",\
  "Sector not found.",\
  "Out of paper.",\
  "Write fault.",\
  "Read fault.",\
  "NVM checksum error.",\
  "Write protected media.",\
  "Media change detected.",\
  "Unknown device.",\
  "Bad sectors on format.",\
  "Insert other disk (request).",\
  "Invalid GEMDOS function number.",\
  "File not found.",\
  "Path not found.",\
  "Handle pool exhausted.",\
  "Access denied.",\
  "Invalid handle.",\
  "Insufficient memory.",\
  "Invalid memory block address.",\
  "Invalid drive specification.",\
  "Not the same drive.",\
  "No more files.",\
  "Record is already locked.",\
  "Invalid lock removal request.",\
  "Argument range error.",\
  "GEMDOS internal error.",\
  "Invalid executable file format.",\
  "Memory block growth failure.",\
  "Too many symbolic links.",\
  "Mount point crossed."
};

static eGdosError g_lastGDOSerror = GDOS_OK;

const uint8 *getLastGemdosError(void)
{
    return getGemdosError(g_lastGDOSerror);
}

const uint8 *getGemdosError(const eGdosError iErr)
{
    uint16 idx=1;

    switch(iErr)
    {
        case GDOS_OK:{
            g_lastGDOSerror = GDOS_OK;
            idx=0;
        } break;
        case GDOS_ERR:{
            g_lastGDOSerror = GDOS_ERR;
            idx=1;
        } break; /*Error*/
        case GDOS_DRIVE_NOT_READY:{
            g_lastGDOSerror = GDOS_DRIVE_NOT_READY;
            idx=2;
        } break;  /*Drive not ready*/
        case GDOS_UNKNOWN_COMMAND:{
            g_lastGDOSerror = GDOS_UNKNOWN_COMMAND;
            idx=3;
        } break;  /*Unknown command*/
        case GDOS_CRC_ERROR:
            g_lastGDOSerror = GDOS_CRC_ERROR;
            idx=4;
        break;  /*CRC (checksum) error*/
        case GDOS_BAD_REQ:
            g_lastGDOSerror=GDOS_BAD_REQ;
            idx=5;
        break;  /*Bad request*/
        case GDOS_SEEK_ERROR:
            g_lastGDOSerror=GDOS_SEEK_ERROR;
            idx=6;
        break;  /*Seek error*/
        case GDOS_UNKNOWN_MEDIA:
            g_lastGDOSerror=GDOS_UNKNOWN_MEDIA;
            idx=7;
        break;  /*Unknown media*/
        case GDOS_SECTOR_NOT_FOUND:
            g_lastGDOSerror=GDOS_SECTOR_NOT_FOUND;
            idx=8;
        break;  /*Sector not found*/
        case GDOS_OUT_OF_PAPER:
            g_lastGDOSerror=GDOS_OUT_OF_PAPER;
            idx=9;
        break;  /*Out of paper*/
        case GDOS_WRITE_FAULT:
            g_lastGDOSerror=GDOS_WRITE_FAULT;
            idx=10;
        break;  /*Write fault*/
        case GDOS_READ_FAULT:
            g_lastGDOSerror=GDOS_READ_FAULT;
            idx=11;
        break;  /*Read fault*/
        case GDOS_NVM_CHESUM_ERR:
            g_lastGDOSerror=GDOS_NVM_CHESUM_ERR;
            idx=12;
        break;  /*NVM checksum error*/
        case GDOS_WRITE_PROTECTED_MEDIA:
            g_lastGDOSerror=GDOS_WRITE_PROTECTED_MEDIA;
            idx=13;
        break;  /*Write protected media*/
        case GDOS_MEDIA_CHANGE_DETECTED:
            g_lastGDOSerror=GDOS_MEDIA_CHANGE_DETECTED;
            idx=14;
        break;  /*Media change detected*/
        case GDOS_UNKNOWN_DEVICE:
            g_lastGDOSerror=GDOS_UNKNOWN_DEVICE;
            idx=15;
        break;  /*Unknown device*/
        case GDOS_BAD_SECTOR_ON_FORMAT:
            g_lastGDOSerror=GDOS_BAD_SECTOR_ON_FORMAT;
            idx=16;
        break;  /*Bad sectors on format*/
        case GDOS_INSERT_OTHER_DISK_REQ:
            g_lastGDOSerror=GDOS_INSERT_OTHER_DISK_REQ;
            idx=17;
        break;  /*Insert other disk (request)*/
        case GDOS_INVALID_GDOS_FUNC_NB:
            g_lastGDOSerror=GDOS_INVALID_GDOS_FUNC_NB;
            idx=18;
        break;  /*Invalid GEMDOS function number*/
        case GDOS_FILE_NOT_FOUND:
            g_lastGDOSerror=GDOS_FILE_NOT_FOUND;
            idx=19;
        break;  /*File not found*/
        case GDOS_PATH_NOT_FOUND:
            g_lastGDOSerror=GDOS_PATH_NOT_FOUND;
            idx=20;
        break;  /*Path not found*/
        case GDOS_HANDLE_POOL_EXHAUSTED:
            g_lastGDOSerror=GDOS_HANDLE_POOL_EXHAUSTED;
            idx=21;
        break;  /*Handle pool exhausted*/
        case GDOS_ACCESS_DENIED:
            g_lastGDOSerror=GDOS_ACCESS_DENIED;
            idx=22;
        break;  /*Access denied*/
        case GDOS_INVALID_HANDLE:
            g_lastGDOSerror=GDOS_INVALID_HANDLE;
            idx=23;
        break;  /*Invalid handle*/
        case GDOS_INSUFFICIENT_MEMORY:
            g_lastGDOSerror=GDOS_INSUFFICIENT_MEMORY;
            idx=24;
        break;  /*Insufficient memory*/
        case GDOS_INVALID_MEM_BLOCK_ADDR:
            g_lastGDOSerror=GDOS_INVALID_MEM_BLOCK_ADDR;
            idx=25;
        break;  /*Invalid memory block address*/
        case GDOS_INVALID_DRIVE_SPEC:
            g_lastGDOSerror=GDOS_INVALID_DRIVE_SPEC;
            idx=26;
        break;  /*Invalid drive specification*/
        case GDOS_NOT_THE_SAME_DRIVE:
            g_lastGDOSerror=GDOS_NOT_THE_SAME_DRIVE;
            idx=27;
        break;  /*Not the same drive*/
        case GDOS_NO_MORE_FILES:
            g_lastGDOSerror=GDOS_NO_MORE_FILES;
            idx=28;
        break;  /*No more files*/
        case GDOS_RECORD_LOCKED:
            g_lastGDOSerror=GDOS_RECORD_LOCKED;
            idx=29;
        break;  /*Record is already locked*/
        case GDOS_INVALID_LOCK_REMOVAL_REQ:
            g_lastGDOSerror=GDOS_INVALID_LOCK_REMOVAL_REQ;
            idx=30;
        break;  /*Invalid lock removal request*/
        case GDOS_ARG_RANGE_ERR:
            g_lastGDOSerror=GDOS_ARG_RANGE_ERR;
            idx=31;
		break;  /*Argument range error*/
        case GDOS_INTERNAL_ERR:
            g_lastGDOSerror=GDOS_INTERNAL_ERR;
            idx=32;
        break;  /*GEMDOS internal error*/
        case GDOS_INVALID_EXE_FMT:
            g_lastGDOSerror=GDOS_INVALID_EXE_FMT;
            idx=33;
        break;  /*Invalid executable file format*/
        case GDOS_MEM_BLOCK_GROWTH_FAIL:
            g_lastGDOSerror=GDOS_MEM_BLOCK_GROWTH_FAIL;
            idx=34;
        break;  /*Memory block growth failure*/
        case GDOS_TOO_MANY_SYMLINKS:
            g_lastGDOSerror=GDOS_TOO_MANY_SYMLINKS;
            idx=35;
        break;  /*Too many symbolic links*/
        case GDOS_MOUNT_POINT_CROSSED:
            g_lastGDOSerror=GDOS_MOUNT_POINT_CROSSED;
            idx=36;
        break;  /*Mount point crossed*/
        default:
            /* unknown error */
            g_lastGDOSerror=GDOS_ERR;
            idx=1;
        break;
    }

 return ((const uint8*)g_arGEMDOSerror[idx]);
}

_DTA *processDta = NULL;
_DTA ourDta = {0};

void initGemdos(void)
{
   processDta = Fgetdta();
   Fsetdta(&ourDta);
}

void deinitGemdos(void)
{
    Fsetdta(processDta);
}


// loads file to specified type memory 
void *loadFile(const uint8 *szFileName, const eMemoryFlag memFlag, uint32 *fileLenght, const eGemdosFileAttrib attrib)
{
    *fileLenght = 0;

    uint16 mode = S_READWRITE;

    switch(attrib)
    {
        case FILE_RO:
        {
            mode = S_READ;
        } break;
        case FILE_WO:
        {
            mode = S_WRITE;
        } break;
        case FILE_RW:
        {
            mode = S_READWRITE;
        } break;
    };

    amTrace("[GEMDOS] load file: %s"NL,szFileName);
    const int32 fileHandle = Fopen( szFileName, mode );

    if(fileHandle>0)
    {
        // file found, get size 
        int32 retVal = Fsfirst(szFileName, 0);

        if(retVal == GDOS_OK)
        {
            const int32 fileLength = ourDta.dta_size;

            // allocate buffer 
            void *pData = gUserMemAlloc(fileLength, memFlag, 0);

            if(pData!=NULL)
            {
                amMemSet(pData,0,fileLength);
                retVal = Fread((uint16)fileHandle, (int32)fileLenght, pData);

                if(retVal<0)
                {
                    if(retVal == GDOS_INVALID_HANDLE)
                    {
                        amPrintf("[GEMDOS] Error: Invalid file handle for %s"NL, szFileName);
                    }
                    else
                    {
                        amPrintf("[GEMDOS] Error: %s (%s)"NL, getGemdosError((int16)retVal), szFileName);
                    }
            
                }
                else
                {
                    if(retVal == (int32)fileLength)
                    {
                        amTrace("[GEMDOS] Closing file handle : [%d] "NL, (uint16)fileHandle);

                        retVal = Fclose((uint16)fileHandle);

                        if(retVal != GDOS_OK)
                        {
                            if(retVal == GDOS_INVALID_HANDLE)
                            {
                                amPrintf("[GEMDOS] Error: Invalid file handle when closing %s"NL, szFileName);
                            }
                            else
                            {
                                amTrace("[GEMDOS] Error closing file handle : [%d] "NL, (uint16)fileHandle, getGemdosError((uint16)retVal));
                            }
                        }

                        *fileLenght = fileLength;
                        return pData;
                    }
                    else
                    {
                        // not all data being read 
                        amPrintf((const char *)"Fatal error, unexpected end of file (%s). Expected %d, found %d bytes."NL, szFileName,fileLenght,retVal);
                        amTrace("[GEMDOS] Read error. Unexpected EOF (%s). Expected %d, found %d bytes."NL, szFileName,fileLenght,retVal);

                        // so we have an error, free up memory 
                        gUserMemFree(pData,0);

                        amTrace("[GEMDOS] Closing file handle : [%d] "NL, (uint16)fileHandle);

                        retVal = Fclose((uint16)fileHandle);

                        if(retVal != GDOS_OK)
                        {
                            if(retVal == GDOS_INVALID_HANDLE)
                            {
                                amPrintf("[GEMDOS] Error: Invalid file handle when closing %s"NL, szFileName);
                            }
                            else
                            {
                                amTrace("[GEMDOS] Error closing file handle : [%d] "NL, (uint16)fileHandle, getGemdosError((uint16)retVal));
                            }
                        }
                    }
                }
            }
            else 
            {
                // no memory available 
                amPrintf("Not enough memory to load %s. Requested: %d bytes"NL, szFileName, fileLength);
                amTrace("Not enough memory to load %s Requested: %d bytes"NL, szFileName, fileLength);
            }
        }
        else
        {
            // file not found, cannot check file size, close file
            amTrace("[GEMDOS] Closing file handle : [%d] "NL, (uint16)fileHandle);

            retVal = Fclose((uint16)fileHandle);

            if(retVal != GDOS_OK)
            {
                if(retVal == GDOS_INVALID_HANDLE)
                {
                    amPrintf("[GEMDOS] Error: Invalid file handle when closing %s"NL, szFileName);
                }
                else
                {
                    amTrace("[GEMDOS] Error closing file handle : [%d] "NL, (uint16)fileHandle, getGemdosError((uint16)retVal));
                }
            }
        } 
    }

 amPrintf("[GEMDOS] Error: %s when opening %s"NL, getGemdosError((uint16)fileHandle), szFileName);
 amTrace("[GEMDOS] Error: %s when opening %s"NL, getGemdosError((uint16)fileHandle), szFileName);           

 return NULL;
}

int32 saveFile(const uint8 *szFileName, const void *memBlock, const uint32 memBlockSize)
{
    const int16 fileHandle = Fcreate( szFileName, 0 );
    int32 iRet = GDOS_OK;    
    
    int32 retVal = AM_OK;

    if(fileHandle>0)
    {
	 amTrace("[GEMDOS] Create file, gemdos handle: %d"NL,fileHandle);

     iRet = Fwrite(fileHandle,memBlockSize,memBlock);
       
     if(iRet == memBlockSize)
     {
        amPrintf("\n%s saved [%d bytes written]."NL,szFileName,iRet);
     }
     else
     {
        amPrintf("\nError: %s saved [%d bytes written]."NL,szFileName, iRet);
     }

     amTrace("[GEMDOS] Closing file handle : [%d] "NL, fileHandle);

     iRet = Fclose(fileHandle);

     if(iRet != GDOS_OK)
     {
        if(iRet == GDOS_INVALID_HANDLE)
        {
            amPrintf("[GEMDOS] Error: Invalid file handle for %s"NL, szFileName);
            retVal = AM_ERR;
        }
        else
        {
            amTrace("[GEMDOS] Error closing file handle : [%d] "NL, fileHandle, getGemdosError((uint16)iRet));
            retVal = AM_ERR;
        }
     }
    }
    else
    {
      // invalid handle
      amPrintf("[GEMDOS] Error: %s (%s)"NL, getGemdosError(fileHandle),szFileName);
	  retVal = AM_ERR;
    }

 return retVal;
}
