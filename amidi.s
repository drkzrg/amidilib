; Copyright 2008, 2009, 2010 Pawel Goralski
; e-mail: pawel.goralski@nokturnal.pl
; This file is part of AMIDILIB.

; AMIDILIB is free software: you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation, either version 3 of the License, or
; (at your option) any later version.

; AMIDILIB is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.

; You should have received a copy of the GNU General Public License
; along with AMIDILIB.  If not, see <http://www.gnu.org/licenses/>.


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










