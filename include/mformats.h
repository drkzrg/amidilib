#ifndef MFORMATS_H
#define MFORMATS_H

/** for internal use, midi file types  */
typedef enum _midiFileTypes{
  T_MIDI0=0,
  T_MIDI1,
  T_MIDI2,
  T_XMIDI,
  T_RMID,  /** RMID little endian M$hit invention */
  T_SMF,
  T_XMF,
  T_SNG,
  T_MUS,	/* e.g Doom MIDI music format */
  T_NOK,    /* custom binary format */
  T_MIDIFTMAX
} eMidiFileTypes;


#endif // MFORMATS_H
