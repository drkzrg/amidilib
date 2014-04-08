;================================================
;    Copyright 2007-2012 Pawel Goralski
;    e-mail: pawel.goralski@nokturnal.pl
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

;export symbols

    xdef _super_on		; self explanatory 
    xdef _super_off		; 

    xdef _turnOffKeyclick	;turns off keyclicks
    xdef _installReplayRout	;initialises replay interrupt TB routine and prepares data
    xdef _deinstallReplayRout	;removes replay routine from system
    
    xdef _MIDIsendBuffer	;buffer with data to send (when used)
    xdef _MIDIbytesToSend	;nb of bytes to send (when used)
    
    xdef _flushMidiSendBuffer	;bypass of Atari XBIOS, writes midi data directly to IKBD
    xdef update			;no underscore, it will be not used from C level atm
				;interrupt routine address
    xdef _tbData		;data value passed to mfp registers to install callback
    xdef _tbMode		;mode value passed to mfp registers to install callback
    xdef _midiOutEnabled	;
    xdef _ymOutEnabled		;
    xdef _bTempoChanged		;flags indicating tempo and time signature change
