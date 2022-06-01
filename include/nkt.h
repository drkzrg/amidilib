#ifndef NKT_H
#define NKT_H

/**  Copyright 2007-2021 Pawel Goralski
    
    This file is part of AMIDILIB.
    See license.txt for licensing information.
*/

#include "vartypes.h"

#include "mdevtype.h"            // device types
#include "timing/mfp.h"
#include "memory/linalloc.h"

typedef enum ENKTTRACKSTATE
{
    // play mode
    NKT_PLAY_ONCE  = (uint16)(0b00000001), // play once if set, loop otherwise

    // track state
    NKT_PS_PLAYING = (uint16)(0b00000010), // playing if set, stopped otherwise
    NKT_PS_PAUSED  = (uint16)(0b00000100), // paused if set
} eNktTrackState;

// custom binary midi replay format
typedef enum ENKTMSGTYPE
{
  NKT_MIDIDATA      = (uint16)(0b00000001),
  NKT_TEMPO_CHANGE  = (uint16)(0b00000010),
  NKT_JUMP          = (uint16)(0b00000100),          //not used atm
  NKT_TRIGGER       = (uint16)(0b00001000),
  NKT_END           = (uint16)(0b00010000),
  NKT_MAX_EVENT = 5
} eNktMsgType;

// Midi events update interval
typedef enum ENKTUPDATEFREQ
{
    NKT_U25HZ=0,
    NKT_U50HZ,
    NKT_U100HZ,
    NKT_U200HZ,
    NKT_UMAX
} eNktUpdateFreq;

// Nkt structure in memory / disc / big endian
typedef struct PACK_ATTR NKTBLOCK  
{
  uint16 msgType;        // msgType
  uint16 blockSize;      // block size (data start: (uint32) eventDataPtr+0, data end: (uint32) eventDataPtr + blockSize(? -1) )
  uint32 bufferOffset;   // offset to data start in linear buffer (eventDataPtr)
} sNktBlock;

StaticAssert(sizeof(sNktBlock)==8, "Invalid sNktBlock size!");

typedef struct PACK_ATTR NKTTEMPO
{
  uint32 tempo;                    // quaternote duration in ms, 500ms default
  uint32 tuTable[NKT_UMAX];        // precalculated timesteps for 50, 100, 200hz updates for given tempo
} sNktTempo;                    // to avoid timestep calculation during runtime

StaticAssert(sizeof(sNktTempo)==20, "Invalid sNktTempo size!");

typedef struct NktTrack
{
    uint32 nbOfBlocks;            // nb of event blocks
    uint32 currentBlockId;        // currently replayed block id 0-xxxx
    uint32 eventsBlockBufferSize; // nb of bytes used for events buffer
    uint32 dataBufferSize;        // nb of bytes used for event's data buffer
    uint32 eventsBlockOffset;     // current track offset relative to eventBlocksPtr ( move it to separate track state? )

    uint32 timeElapsedFrac;	   // sequence elapsed time fraction
    uint32 timeElapsedInt;		   // sequence elapsed time

    uint8 *eventBlocksPtr;        // pointer to start of events block
    uint8 *eventDataPtr;          // pointer to start of event's data block

    LinearBufferAllocator lbDataBuffer;  // linear buffer for event data info
    LinearBufferAllocator lbEventsBuffer; // linear buffer for events block info
} sNktTrack;


typedef struct NktSeq
{
    uint16 version;               // version
    uint16 timeDivision;          // track time division
    uint16 currentUpdateFreq;     // as in eNktUpdateFreq enum, indicates midi engine update interval
    uint16 nbOfTracks;            // number of tracks
    uint16 sequenceState;         // bitfield with sequence state
    sNktTempo defaultTempo;	   // initial tempo, quaternote duration in ms, 500ms default/ current bpm, precalculated timesteps
    sNktTempo currentTempo;    // current tempo: quaternote duration in ms, 500ms default/ current bpm, precalculated timesteps

    uint32 timeStep;              // sequence timestep
    sNktTrack *pTracks;        // array of tracks
} sNktSeq;

#define ID_NKT 0x4E4F4B54  /*('N','O','K','T')*/
#define NKT_VERSION ((uint16)4)

// stuff for file reading
// binary header, big endian

typedef struct PACK_ATTR NktHd
{
    uint32 id;                         // always ID_NKT
    uint16 division;                   // time division
    uint16 version;                    // format version
    uint16 nbOfTracks;                 // number of tracks
} sNktHd;

StaticAssert(sizeof(sNktHd)==10, "Invalid sNktHd size!");


typedef struct PACK_ATTR NKTTRACKINFO
{
    uint32 nbOfBlocks;                 // nb of event blocks in file
    uint32 eventsBlockBufSize;         // (event vlq delta * NbOfBlocks) +  ( NbOfBlocks * sizeof(sNktBlk_t) )
    uint32 eventsBlockPackedSize;      // if packed, size of packed data, contigous event block, 0 otherwise
    uint32 eventDataBufSize;           // nb of bytes for data in event blocks
    uint32 eventDataBlockPackedSize;   // if packed, size of packed data, contigous event data block, 0 otherwise
} sNktTrackInfo;

StaticAssert(sizeof(sNktTrackInfo)==20, "Invalid sNktTrackInfo size!");


// file layout:
// + sNktHd
// + nbOfTracks*sNktTrackInfo
// + track data[]
//              :[eventBlock]
//              :[eventData]
// + ...
// + track data[nbOfTracks]


/////////////////////////////////////////////

/** library initialisation, setup of extrernal device on specified channel */
void NktInit(const eMidiDeviceType devType, const uint8 channel);

/** library cleanup / deinitialisation */
void NktDeinit(void);

/** installs update sequence, single/multitrack variant hooked to selected timer type */
AM_EXTERN void NktInstallReplayRout(void);

/** installs update sequence, single/multitrack variant not hooked to timer */
AM_EXTERN void NktInstallReplayRoutNoTimers(void);

/** deinstalls sequence replay routine installed with installReplayRout()  */
AM_EXTERN void NktDeinstallReplayRout(void);

/** returns currently loaded/active nkt sequence pointer */
sNktSeq * const getActiveNktSequence(void);

void initNktSequence(sNktSeq *seq, uint16 initialState, Bool bInstallUpdate);

sNktSeq *loadNktSequence(const uint8 *filepath);

void destroyNktSequence(sNktSeq *pSeq);

// replay control
Bool isNktSequencePlaying(void);
void stopNktSequence(void);
void pauseNktSequence(void);
void playNktSequence(void);
void switchNktReplayMode(void);

void setMidiMasterVolume(const uint8 vol);
void setMidiMasterBalance(const uint8 bal);

uint8 getMidiMasterVolume(void);
uint8 getMidiMasterBalance(void);

#ifdef DEBUG_BUILD
//debug helpers
#ifdef MANUAL_STEP
void initNktSequenceManual(sNktSeq *pSeq, uint16 initialState);
#endif
void printNktSequenceState(void);
const uint8 *getEventTypeName(const uint16 type);
#endif

#endif // NKT_H
