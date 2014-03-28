#ifndef SEQ2NKT_H
#define SEQ2NKT_H

#include <amidiseq.h> //todo remove this dependency
#include "nkt.h"


S32 Seq2Nkt(const sSequence_t *pSeq, U8* out, const U8 *pOutFileName, const BOOL bCompress);


#endif // SEQ2NKT_H
