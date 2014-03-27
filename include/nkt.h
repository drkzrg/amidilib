#ifndef NKT_H
#define NKT_H

#include <c_vars.h>
#include <amidiseq.h>

// custom binary midi replay format
typedef enum{
  NKT_MIDIDATA=0,
  NKT_TEMPO_CHANGE,
  NKT_JUMP,  //not used atm
  NKT_TRIGGER,
  NKT_END,
  NKT_MAX_EVENT
} eNktMsgType;

typedef struct NktBlock_t{
  U32 delta;
  eNktMsgType msgType;
  U32 blockSize;
  U8 *pData;
} sNktBlock_t;

// binary header, big endian
typedef struct __attribute__((packed)) NktHd{
    U32 id;            // always ID_NKT
    U32 NbOfBlocks;    // nb of event blocks in file
    BOOL bPacked;      // blocks, are data blocks LZO packed?
    U16 division;      // timeDivision
    U16 version;       // format version
                       // TODO: add info / description block (?)
} sNktHd;

#define ID_NKT 0x4E4F4B54  /*('N','O','K','T')*/

S32 Seq2Nkt(const sSequence_t *pSeq, U8* out, const U8 *pOutFileName, const BOOL bCompress);


#endif // NKT_H
