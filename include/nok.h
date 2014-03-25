#ifndef NOK_H
#define NOK_H

#include <c_vars.h>
#include <amidiseq.h>

// custom binary midi replay format
typedef enum{
  NOK_MIDIDATA=0,
  NOK_TEMPO_CHANGE,
  NOK_JUMP,  //not used atm
  NOK_TRIGGER,
  NOK_END,
  NOK_MAX_EVENT
} eNokMsgType;

typedef struct NokBlock_t{
  U32 delta;
  eNokMsgType msgType;
  U32 blockSize;
  U8 *pData;
} sNokBlock_t;

// binary header, big endian
typedef struct __attribute__((packed)) NokHd{
    U32 id;            // always ID_NOK
    U32 NbOfBlocks;    // nb of event blocks in file
    BOOL bPacked;      // blocks, are data blocks LZO packed?
    U16 division;      // timeDivision
    U16 version;       // format version
                       // TODO: add info/description block (?)
} sNokHd;

#define ID_NOK 0x4E4F4B54  /*('N','O','K','T')*/

S32 Seq2Nok(const sSequence_t *pSeq, U8* out, const U8 *pOutFileName, const BOOL bCompress);


#endif // NOK_H
