#ifndef NKT_H
#define NKT_H

#include <c_vars.h>
#include "memory/linalloc.h"

typedef enum{
    // play mode
    NKT_PLAY_ONCE  = 0b00000001, // play once if set, loop otherwise

    // track state
    NKT_PS_PLAYING = 0b00000010, // playing if set, stopped otherwise
    NKT_PS_PAUSED  = 0b00000100, // paused if set
}eNktTrackState;


// custom binary midi replay format
typedef enum{
  NKT_MIDIDATA      = 0b00000001,
  NKT_TEMPO_CHANGE  = 0b00000010,
  NKT_JUMP          = 0b00000100,          //not used atm
  NKT_TRIGGER       = 0b00001000,
  NKT_END           = 0b00010000,
  NKT_MAX_EVENT = 5
} eNktMsgType;

// Nkt structure in memory
typedef struct NktBlock_t{
  U32 delta;
  U16 msgType;
  U16 blockSize;
  U8 *pData;
} sNktBlock_t;

typedef struct NktSeq{
    U16 version;               // version
    U16 timeDivision;          // time division
    U32 defaultTempo;		   // quaternote duration in ms, 500ms default
    U32 lastTempo;             // quaternote duration in ms, 500ms default
    U32 currentBPM;	           // beats per minute (60 000000 / currentTempo)
    U32 timeElapsedFrac;	   // track elapsed time
    U32 timeElapsedInt;		   // track elapsed time
    U32 timeStep;              // current track's timestep
    U16 sequenceState;         // bitfield with sequence state
    U32 dataBufferSize;        // nb of bytes used for data buffer
    U8 *pEventDataBuffer;
    tLinearBuffer dataBuffer;  // custom data buffer info

    U32 NbOfBlocks;            // nb of event blocks
    U32 currentBlockId;        // currently replayed block id 0-xxxx
    sNktBlock_t *pEvents;      // eventStart
    tLinearBuffer eventBuffer; // custom event buffer info

} sNktSeq;

#define ID_NKT 0x4E4F4B54  /*('N','O','K','T')*/

// stuff for file read
// binary header, big endian
typedef struct __attribute__((packed)) NktHd{
    U32 id;            // always ID_NKT
    U32 NbOfBlocks;    // nb of event blocks in file
    U32 NbOfBytesData; // nb of bytes for data in event blocks
    BOOL bPacked;      // blocks, are data blocks LZO packed?
    U16 division;      // timeDivision
    U16 version;       // format version
                       // TODO: add info / description block (?)
} sNktHd;

// binary header, big endian
typedef struct __attribute__((packed)) NktBlk{
    U16 msgType;
    U16 blockSize;
} sNktBlk;
/////////////////////////////////////////////

void getCurrentSequence(sNktSeq **pSeq);
void initSequence(sNktSeq *seq, U16 initialState);

sNktSeq *loadSequence(const U8 *filepath);
void destroySequence(sNktSeq *pSeq);

// replay control
BOOL isSequencePlaying(void);
void stopSequence(void);
void pauseSequence();
void playSequence(void);
void switchReplayMode(void);

//debug helpers
void initSequenceManual(sNktSeq *pSeq, U16 initialState); //todo remove in final build
void printNktSequenceState();
const U8 *getEventTypeName(U16 type);

#endif // NKT_H
