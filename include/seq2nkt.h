#ifndef SEQ2NKT_H
#define SEQ2NKT_H

#include "amidiseq.h"
#include "nkt.h"

int32 Seq2NktFile(const sSequence_t *pSeq, const uint8 *pOutFileName, const Bool bCompress);

#endif // SEQ2NKT_H
