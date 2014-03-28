#ifndef SEQ2NKT_H
#define SEQ2NKT_H

#include <amidiseq.h>
#include "nkt.h"


S32 Seq2NktFile(const sSequence_t *pSeq, const U8 *pOutFileName, const BOOL bCompress);

#endif // SEQ2NKT_H
