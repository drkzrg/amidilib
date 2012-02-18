
#    Copyright 2007-2010 Pawel Goralski
#    e-mail: pawel.goralski@nokturnal.pl
#    This file is part of AMIDILIB.
#    See license.txt for licensing information.

#############################################################################################################################
# additional defines for EXTRADEFINES: 
#############################################################################################################################
# DEBUG_BUILD - enables debug build
# DEBUG_FILE_OUTPUT enables log output to files (works only if DEBUG_BUILD is defined)  
# DEBUG_CONSOLE_OUTPUT enables log output to console (works only if DEBUG_BUILD is defined)  
# DEBUG_MEM logs memory function calling (works only if DEBUG_BUILD && (DEBUG_FILE_OUTPUT||DEBUG_CONSOLE_OUTPUT) is enabled)
# FORCE_MALLOC forces use of libc malloc() for memory allocation, not native functions
# TIME_CHECK_PORTABLE if set time measuring is based on stdlib functions
# PORTABLE build portable,platform independent version 
# MIDI_PARSER_DEBUG output midi parsing (works only if DEBUG_BUILD && (DEBUG_FILE_OUTPUT||DEBUG_CONSOLE_OUTPUT) is enabled)
# STRUCT_PACK enables structure packing if defined (for now only gcc compatible) 
# MIDI_PARSER_TEST outputs loaded and parsed midi file in human readable form
# IKBD_MIDI_SEND_DIRECT all Midiws() calls will be replaced with low level function that writes to hardware.

EXTRADEFINES = 
#-DDEBUG_BUILD -DDEBUG_MEM -DDEBUG_FILE_OUTPUT -DMIDI_PARSER_DEBUG -DMIDI_PARSER_TEST
#-DDEBUG_BUILD -DDEBUG_FILE_OUTPUT -DDEBUG_CONSOLE_OUTPUT 

#target atari, other
ifeq ($(TARGET),atari)
INCLUDES=-I./ -I./include -I/usr/m68k-atari-mint/include -I./include/lzo -I./include/ym2149
CC=m68k-atari-mint-gcc
GAS=m68k-atari-mint-as
STRIP=m68k-atari-mint-strip -s
STACK=m68k-atari-mint-stack
MACHINE=-m68000	
LD_EXTRA=-L/usr/m68k-atari-mint/lib

# stack settings for all apps
STACK_SIZE=32768
SET_STACK=$(STACK) -S$(STACK_SIZE)
BIN_EXT=.tos
BIN_EXT2=.ttp
else
INCLUDES=-I./ -I./include -I./include/lzo -I./include/ym2149
CC=gcc
GAS=as
STRIP=strip -s
STACK=stack
PORTABLE=1
MACHINE=	
LD_EXTRA=
SET_STACK=
BIN_EXT=
endif


ifeq ($(PORTABLE),1)
DEFINES = $(EXTRADEFINES) -DPORTABLE -DTIME_CHECK_PORTABLE
else
DEFINES = $(EXTRADEFINES) -DFORCE_MALLOC 
endif

CFLAGS += -std=c99 $(MACHINE) $(INCLUDES) -g -Wall -fsigned-char -fomit-frame-pointer -Wl,--stack,$(STACK_SIZE) $(DEFINES)
LDFLAGS +=  $(MACHINE) $(LD_EXTRA) -Wl,--traditional-format 

ASM = vasmm68k_mot
ASMFLAGS += -Faout -quiet -x -m68000 -spaces -showopt -no-opt
EXE = amidi$(BIN_EXT2)

# ym2149 test output program 
YM_TEST_EXE = ym2149$(BIN_EXT)

# midi test output program
MIDI_TEST_EXE = midiTest$(BIN_EXT)

#timing test output program
TIMING_TEST_EXE = timTest$(BIN_EXT)

#copies output binary to emulator folder ready to launch
ST_HD_PATH=$(HOME)/STEEM/HD/TUNES
SHARED_DIR_PATH=$(HOME)/Pulpit/shared

ifeq ($(PORTABLE),1)
SRCS = ./src/main.c ./src/twisterm.c ./src/mt32.c ./src/midiseq.c ./src/list.c ./src/iff.c ./src/fmio.c ./src/cm_500.c ./src/cm_32l.c ./src/c_vars.c ./src/amidilib.c ./src/tbl_stat.c ./src/minilzo.c ./src/memory.c ./src/amlog.c ./src/midi_rep.c
OBJECTS = ./src/main.o ./src/twisterm.o ./src/mt32.o ./src/midiseq.o ./src/list.o ./src/iff.o ./src/fmio.o ./src/cm_500.o ./src/cm_32l.o ./src/c_vars.o ./src/amidilib.o ./src/tbl_stat.o ./src/minilzo.o ./src/memory.o ./src/amlog.o ./src/midi_rep.o
else
SRCS = ./src/main.c ./src/twisterm.c ./src/mt32.c ./src/midi_rep.c ./src/midiseq.c ./src/list.c ./src/iff.c ./src/fmio.c ./src/cm_500.c ./src/cm_32l.c ./src/c_vars.c ./src/amidilib.c ./src/tbl_stat.c ./src/minilzo.c ./src/memory.c ./src/amlog.c ./src/mfp.c 
OBJECTS = ./src/main.o ./src/twisterm.o ./src/mt32.o ./src/midi_rep.o ./src/midiseq.o ./src/list.o ./src/iff.o ./src/fmio.o ./src/cm_500.o ./src/cm_32l.o ./src/c_vars.o ./src/amidilib.o ./src/tbl_stat.o ./src/minilzo.o ./src/memory.o ./src/amlog.o ./src/mfp.o
endif

ifeq ($(PORTABLE),1)
MIDITEST_SRCS = ./src/midiTest.c ./src/c_vars.c ./src/mt32.c ./src/midiseq.c ./src/fmio.c ./src/cm_500.c ./src/cm_32l.c ./src/amidilib.c ./src/tbl_stat.c ./src/list.c ./src/memory.c ./src/amlog.c
MIDITEST_OBJECTS = ./src/midiTest.o ./src/c_vars.o ./src/midiseq.o ./src/fmio.o ./src/cm_500.o ./src/cm_32l.o ./src/amidilib.o ./src/tbl_stat.o ./src/list.o ./src/memory.o ./src/amlog.o
else
MIDITEST_SRCS = ./src/midiTest.c ./src/c_vars.c ./src/mt32.c ./src/midiseq.c ./src/fmio.c ./src/cm_500.c ./src/cm_32l.c ./src/amidilib.c ./src/tbl_stat.c ./src/list.c ./src/memory.c ./src/amlog.c  ./src/mfp.c 
MIDITEST_OBJECTS = ./src/midiTest.o ./src/c_vars.o ./src/midiseq.o ./src/fmio.o ./src/cm_500.o ./src/cm_32l.o ./src/amidilib.o ./src/tbl_stat.o ./src/list.o ./src/memory.o ./src/amlog.o  ./src/mfp.o
endif

ifeq ($(PORTABLE),1)
YM_TEST_SRCS = ./src/ymTest.c ./src/c_vars.c ./src/ym2149.c ./src/memory.c ./src/amlog.c
YM_TEST_OBJECTS = ./src/ymTest.o ./src/c_vars.o ./src/ym2149.o ./src/memory.o ./src/amlog.o
else
YM_TEST_SRCS = ./src/ymTest.c ./src/c_vars.c ./src/ym2149.c ./src/memory.c ./src/amlog.c ./src/mfp.c ./src/fmio.c
YM_TEST_OBJECTS = ./src/ymTest.o ./src/c_vars.o ./src/ym2149.o ./src/memory.o ./src/amlog.o ./src/mfp.o ./src/fmio.o
endif

ifeq ($(PORTABLE),1)
TIMING_TEST_SRCS = ./src/timTest.c ./src/ym2149.c ./src/c_vars.c ./src/mt32.c ./src/midiseq.c ./src/fmio.c ./src/cm_500.c ./src/cm_32l.c ./src/amidilib.c ./src/tbl_stat.c ./src/list.c ./src/memory.c ./src/amlog.c
TIMING_TEST_OBJECTS = ./src/timTest.o ./src/c_vars.o ./src/ym2149.o ./src/mt32.o ./src/midiseq.o ./src/fmio.o ./src/cm_500.o ./src/cm_32l.o ./src/amidilib.o ./src/tbl_stat.o ./src/list.o ./src/memory.o ./src/amlog.o
else
TIMING_TEST_SRCS = ./src/timTest.c ./src/ym2149.c ./src/c_vars.c ./src/mt32.c ./src/midiseq.c ./src/fmio.c ./src/cm_500.c ./src/cm_32l.c ./src/amidilib.c ./src/tbl_stat.c ./src/list.c ./src/memory.c ./src/amlog.c ./src/mfp.c
TIMING_TEST_OBJECTS = ./src/timTest.o ./src/c_vars.o ./src/ym2149.o ./src/mt32.o ./src/midiseq.o ./src/fmio.o ./src/cm_500.o ./src/cm_32l.o ./src/amidilib.o ./src/tbl_stat.o ./src/list.o ./src/memory.o ./src/amlog.o ./src/mfp.o
endif

ifeq ($(PORTABLE),1)
$(EXE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@ -lm 
else
$(EXE): $(OBJECTS) amidi.o ikbd_asm.o
	$(CC) $(LDFLAGS) amidi.o $(OBJECTS) ikbd.o -o $@ -lgem -lm
endif
	echo "Setting AMIDI.TTP stack to: " $(STACK_SIZE)
ifeq ($(PORTABLE),0)
	$(SET_STACK) $(EXE)
endif
	echo "Stripping symbols."
	$(STRIP) $(EXE)
	echo "Copying AMIDI.TTP binary to emulator/shared directory."
	cp $(EXE) $(ST_HD_PATH)
	cp $(EXE) $(SHARED_DIR_PATH)
ifeq ($(PORTABLE),1)
$(YM_TEST_EXE): $(YM_TEST_OBJECTS) 
	$(CC) $(LDFLAGS) $(YM_TEST_OBJECTS) -o $@ -lm 
else
$(YM_TEST_EXE): $(YM_TEST_OBJECTS) amidi.o 
	$(CC) $(LDFLAGS) $(YM_TEST_OBJECTS) amidi.o -o $@ -lm 
endif
	echo "Stripping symbols"
	$(STRIP) $(YM_TEST_EXE)
	echo "Copying ym2149 test program to emulator directory."
	cp $(YM_TEST_EXE) $(ST_HD_PATH)
	cp $(YM_TEST_EXE) $(SHARED_DIR_PATH)

ifeq ($(PORTABLE),1)
$(MIDI_TEST_EXE): $(MIDITEST_OBJECTS)  
	$(CC) $(LDFLAGS) $(MIDITEST_OBJECTS) -o $@ -lm
else
$(MIDI_TEST_EXE): $(MIDITEST_OBJECTS) amidi.o ikbd_asm.o   
	$(CC) $(LDFLAGS) $(MIDITEST_OBJECTS) amidi.o ikbd.o -o $@ -lm

endif
	echo "Stripping symbols"
	$(STRIP) $(MIDI_TEST_EXE)
	echo "Copying midi output test program to emulator/shared directory."
	cp $(MIDI_TEST_EXE) $(ST_HD_PATH)
	cp $(MIDI_TEST_EXE) $(SHARED_DIR_PATH)

ifeq ($(PORTABLE),1)
$(TIMING_TEST_EXE): $(TIMING_TEST_OBJECTS)   
	$(CC) $(LDFLAGS) $(TIMING_TEST_OBJECTS) -o $@ -lm 
else
$(TIMING_TEST_EXE): $(TIMING_TEST_OBJECTS) amidi.o testReplay.o ikbd_asm.o   
	$(CC) $(LDFLAGS) $(TIMING_TEST_OBJECTS) amidi.o testReplay.o ikbd.o  -o $@ -lgem -lm 
endif
	echo "Stripping symbols"
	$(STRIP) $(TIMING_TEST_EXE)
	echo "Copying midi delta timing test program to emulator/shared directory."
	cp $(TIMING_TEST_EXE) $(ST_HD_PATH)
	cp $(TIMING_TEST_EXE) $(SHARED_DIR_PATH)

ifneq ($(PORTABLE),1)
amidi.o:	./src/amidi.s
		$(ASM) ./src/amidi.s $(ASMFLAGS) -o amidi.o

testReplay.o:	./src/testReplay.s
		$(ASM) ./src/testReplay.s $(ASMFLAGS) -o testReplay.o

ikbd_asm.o:	./src/ikbd.S
		$(GAS) $(MACHINE) ./src/ikbd.S -o ikbd.o
endif

$(OBJECTS): %.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(YM_TEST_OBJECTS): %.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(MIDITEST_OBJECTS): %.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TIMING_TEST_OBJECTS): %.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@


ym: $(YM_TEST_SRCS) $(YM_TEST_EXE)	
midi: $(MIDITEST_SRCS) $(MIDI_TEST_EXE)	
timing: $(TIMING_TEST_SRCS) $(TIMING_TEST_EXE)

all: $(EXE) $(YM_TEST_EXE) $(MIDI_TEST_EXE) $(TIMING_TEST_EXE)
	
clean:
	rm -rf *o $(EXE)
	rm -rf *o $(YM_TEST_EXE)
	rm -rf *o $(MIDI_TEST_EXE)
	rm -rf *o $(TIMING_TEST_EXE)

.PHONY: default clean

