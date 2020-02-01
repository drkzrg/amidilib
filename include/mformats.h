#ifndef MFORMATS_H
#define MFORMATS_H

/** for internal use, midi file types  */
typedef enum EMIDIFILETYPES 
{
  T_MIDI0 = 0,
  T_MIDI1,
  T_MIDI2,
  T_XMIDI,
  T_RMID,  /** RMID little endian M$hit invention */
  T_SMF,
  T_XMF,
  T_SNG,
  T_MUS,	/* e.g Doom MIDI music format */
  T_NKT,    /* NKT custom binary format */
  T_MIDIFTMAX,
  T_INVALID = -1,    
  T_UNSUPPORTED = -2,    
  
} eMidiFileType;

#endif // MFORMATS_H
