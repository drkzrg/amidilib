
/**  Copyright 2007-2019 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#include "c_vars.h"

/* Compiler sanity check */

void compilerSanityCheck(){

 /* check compiler sanity */
 /* if something below will be not right then compiler will moan ;)) ..*/
 /* In this case check your compiler docs with typedefs from C_VARS.H */

    STATIC_ASSERT((sizeof(uint8)==1),"static assert: incorrect uint8 size");
    STATIC_ASSERT(((uint8)-1)>0,"static assert: uint8 not unsigned!");
    STATIC_ASSERT(sizeof(uint16)==2,"static assert: incorrect uint16 size");
    STATIC_ASSERT(((uint16)-1)>0,"static assert: uint16 not unsigned!");
    STATIC_ASSERT(sizeof(uint32)==4,"static assert: incorrect uint32 size");
    STATIC_ASSERT((uint32)-1>0,"static assert: incorrect uint32 size not unsigned");
    STATIC_ASSERT(sizeof(int32)==4,"static assert: incorrect int32 size!");
    STATIC_ASSERT((int32)-1==-1,"static assert: incorrect int32 not signed!");
    STATIC_ASSERT(sizeof(fp32)==4,"static assert: incorrect fp32 size!");
    STATIC_ASSERT(sizeof(fp64)==8,"static assert: incorrect fp64 size!");
}

// check processor endianess ( processor sanity check ;) )
// returns:
// TRUE - for little endian, x86, PS2
// FALSE / for big endian - // Big Endian - GameCube, Atari

const bool checkEndianess(){
  uint16 word16=1;

 if (*(uint8 *)&word16==1){
        // little endian - x86, PS2
        return TRUE;
    } else{
        // Big Endian - GameCube, Atari
        return FALSE;
    }
}
