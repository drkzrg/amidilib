;   Copyright 2007-2010 Pawel Goralski
;   e-mail: pawel.goralski@nokturnal.pl
;   This file is part of AMIDILIB.
;   See license.txt for licensing information.

  XDEF am_writeVarLen
  XDEF am_readVarLen
  XDEF am_send_midi_cmd
  
  
    text
am_readVarLen:
    move.l  d0,d1
    moveq   #0,d0
    ;result now in d0 
    ;VLC value copied in d1

    ;results in d0.l */
    RTS


am_writeVarLen:
  ; value to write in d0.l


    RTS

;a0 - address of midi buffer
am_send_midi_cmd:
  move.b  (a0)+,d0
  btst    #1,$fffffc04          ;is data reg empty?
  beq.s   am_send_midi_cmd          ;no, wait for empty
  move.b  d0,$fffffc06          ;write to MIDI data reg
  rts                             ;return










