#ifndef NKT_H
#define NKT_H

#include <c_vars.h>

#include "mdevtype.h"            // device types
#include "timing/mfp.h"
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

// Midi events update interval
typedef enum {
    NKT_U25HZ=0,
    NKT_U50HZ,
    NKT_U100HZ,
    NKT_U200HZ,
    NKT_UMAX
} eNktUpdateFreq;

// Nkt structure in memory
typedef struct NktBlock_t{
  U32 delta;
  U16 msgType;
  U16 blockSize;
  U8 *pData;
} sNktBlock_t;

typedef struct NktTempo{
  U32 tempo;                    // quaternote duration in ms, 500ms default
  U32 tuTable[NKT_UMAX];        // precalculated timesteps for 50, 100, 200hz updates for given tempo
} sNktTempo;                    // to avoid timestep calculation during runtime

typedef struct NktSeq{
    U16 version;               // version
    U16 timeDivision;          // track time division
    U16 currentUpdateFreq;     // as in eNktUpdateFreq enum, indicates midi engine update interval (TODO maybe we move it to global space / engine space)
    sNktTempo defaultTempo;	   // initial tempo, quaternote duration in ms, 500ms default/ current bpm, precalculated timesteps
    sNktTempo currentTempo;    // current tempo: quaternote duration in ms, 500ms default/ current bpm, precalculated timesteps
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
    BOOL bPacked;      // blocks, are data LZO packed?
    U32 bytesPacked;   // bytes packed if (bPacked==TRUE)
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

void NktInit(const eMidiDeviceType devType, const U8 channel);
void NktDeinit();

/** installs update sequence, single/multitrack variant hooked to selected timer type */
extern void NktInstallReplayRout();

/** deinstalls sequence replay routine installed with installReplayRout()  */
extern void NktDeinstallReplayRout();

void getCurrentSequence(sNktSeq **pSeq);
void initSequence(sNktSeq *seq, U16 initialState, BOOL bInstallUpdate);

sNktSeq *loadSequence(const U8 *filepath);
void destroySequence(sNktSeq *pSeq);

// replay control
BOOL isSequencePlaying(void);
void stopSequence(void);
void pauseSequence();
void playSequence(void);
void switchReplayMode(void);

#ifdef DEBUG_BUILD
//debug helpers
#ifdef MANUAL_STEP
void initSequenceManual(sNktSeq *pSeq, U16 initialState); //todo remove in final build
#endif
void printNktSequenceState();
const U8 *getEventTypeName(U16 type);
#endif

#endif // NKT_H
