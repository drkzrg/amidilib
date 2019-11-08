;================================================
;    Copyright 2007-2019 Pawel Goralski
;    
;    This file is part of AMIDILIB.
;    See license.txt for licensing information.
;================================================

;constants
AMIDI_MAX_TRACKS	equ	65536
MIDI_CLOCK_ENABLE	equ	0
MIDI_SENDBUFFER_SIZE 	equ	32*1024

MIDI_CLOCK_BYTE		equ 	$F8	;one byte MIDI realtime message (MIDI CLOCK)
MIDI_START		equ 	$FA	;one byte MIDI realtime message (starts MIDI clock sync)
MIDI_STOP		equ 	$FC	;one byte MIDI realtime message (stops MIDI clock sync)
AUTO_INT_ENABLE		equ	0	;enable auto end of interrupt mode


; export symbols
    xdef _super_on		; self explanatory
    xdef _super_off		;

    xdef _turnOffKeyclick	;turns off keyclicks

    xdef _installReplayRoutGeneric	;initialises replay interrupt TB routine and prepares data
    xdef _deinstallReplayRoutGeneric	;removes replay routine from system

    xdef _installReplayRout	;initialises replay interrupt routine (single / multitrack) on selected timer (this should be moved somewhere)
    xdef _deinstallReplayRout	;removes replay routine from system

    xdef _MIDIsendBuffer	;buffer with data to send (when used)
    xdef _MIDIbytesToSend	;nb of bytes to send (when used)
    
    xdef _flushMidiSendBuffer	;bypass of Atari XBIOS, writes midi data directly to IKBD
    xdef update			;no underscore, it will be not used from C level atm
				;interrupt routine address
    xdef _midiOutEnabled	;
    xdef _ymOutEnabled		;
    xdef _bTempoChanged		;flags indicating tempo and time signature change
    xdef _NktMidiUpdateHook     ;nkt replay midi update hook to insert into system Timer C interrupt (200Hz)

    xdef _installMidiResetHandler ;adds custom reset handler

;timer related, internal data
; TiB
    xdef stopTiB
    xdef updateTiB
    xdef finishTiB
; TiC
    xdef vectorTiC
    xdef stopTiC
    xdef updateTiC
    xdef finishTiC
;
    xdef tData                  ; Timer mode / data
    xdef tMode                  ;

    xdef updateStepRout             ; function pointer to update routine (single / multitrack)
    xdef stopTimerIntPtr        ; function pointer to timer stop function
    xdef updateTimerIntPtr      ; function pointer to timer update / setup function
    xdef finishTimerIntPtr      ; function pointer to signal timer finish

    xdef _isMultitrackReplay     ; flag indicates if multitrack / single track replay is installed
    xdef timerReplayType:       ; currently installed midi sequence handler type (TiB/TiC etc.)
    xdef oldVector              ; old timer vector replaced by us
    xdef midiIntCounter         ; interruptCounter (used in TiC update variant)
    xdef midiIntHz              ; if(midiIntHz-1==midiIntCounter) midi internal buffer update is called (used in TiC only)
