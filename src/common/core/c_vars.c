
/**  Copyright 2007-2010 Pawel Goralski
    e-mail: pawel.goralski@nokturnal.pl
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#include <assert.h>
#include "c_vars.h"

/* Compiler sanity check */

void compilerSanityCheck(){

 /* check compiler sanity */
 /* if something below will be not right then compiler will moan ;)) ..*/
 /* In this case check your compiler docs with typedefs from C_VARS.H */

    STATIC_ASSERT((sizeof(U8)==1),"static assert: incorrect U8 size");
    STATIC_ASSERT(((U8)-1)>0,"static assert: U8 not unsigned!");
    STATIC_ASSERT(sizeof(U16)==2,"static assert: incorrect U16 size");
    STATIC_ASSERT(((U16)-1)>0,"static assert: U16 not unsigned!");
    STATIC_ASSERT(sizeof(U32)==4,"static assert: incorrect U32 size");
    STATIC_ASSERT((U32)-1>0,"static assert: incorrect U32 size not unsigned");
    STATIC_ASSERT(sizeof(S32)==4,"static assert: incorrect S32 size!");
    STATIC_ASSERT((S32)-1==-1,"static assert: incorrect S32 not signed!");
    STATIC_ASSERT(sizeof(FP32)==4,"static assert: incorrect FP32 size!");
    STATIC_ASSERT(sizeof(FP64)==8,"static assert: incorrect FP64 size!");
}

// check processor endianess ( processor sanity check ;) )
// returns:
// TRUE - for little endian, x86, PS2
// FALSE / for big endian - // Big Endian - GameCube, Atari

const BOOL checkEndianess(){
  U16 uint16=1;

 if (*(U8 *)&uint16==1){
        // little endian - x86, PS2
        return TRUE;
    } else{
        // Big Endian - GameCube, Atari
        return FALSE;
    }
}
