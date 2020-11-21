
/**  Copyright 2007-2020 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#include "c_vars.h"

/* Compiler sanity check */
void compilerSanityCheck(void)
{
 /* check compiler sanity */
 /* if something below will be not right then compiler will moan ;)) ..*/
 /* In this case check your compiler docs with typedefs from C_VARS.H */

    StaticAssert((sizeof(uint8)==1),"incorrect uint8 size");
    StaticAssert((((uint8)-1)>0),"uint8 not unsigned!");
    StaticAssert((sizeof(uint16)==2),"incorrect uint16 size");
    StaticAssert((((uint16)-1)>0),"uint16 not unsigned!");
    StaticAssert((sizeof(uint32)==4),"incorrect uint32 size");
    StaticAssert(((uint32)-1>0),"incorrect uint32 size not unsigned");
    StaticAssert((sizeof(int32)==4),"incorrect int32 size!");
    StaticAssert(((int32)-1==-1),"incorrect int32 not signed!");
    StaticAssert((sizeof(fp32)==4),"incorrect fp32 size!");
    StaticAssert((sizeof(fp64)==8),"incorrect fp64 size!");
}

// check processor endianess ( processor sanity check ;) )
// returns:
// TRUE - for little endian, x86, PS2
// FALSE / for big endian - // Big Endian - GameCube, Atari

const bool checkEndianess(void)
{
  uint16 word16=1;

 if (*(uint8 *)&word16==1){
        // little endian - x86, PS2
        return TRUE;
    } else{
        // Big Endian - GameCube, Atari
        return FALSE;
    }
}
