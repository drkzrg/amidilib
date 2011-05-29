
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

EXTRADEFINES = 
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
STACK_SIZE=8192
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

CFLAGS += -std=c99 -g $(MACHINE) $(INCLUDES) -Wall -Wpadded -Wpacked -fsigned-char -fomit-frame-pointer -Wl,--stack,$(STACK_SIZE) $(DEFINES)
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
SRCS = main.c twisterm.c mt32.c midi_cmd.c midi_send.c midiseq.c list.c iff.c fmio.c cm_500.c cm_32l.c c_vars.c amidilib.c tbl_stat.c minilzo.c memory.c amlog.c midi_rep.c
OBJECTS = main.o twisterm.o mt32.o midi_cmd.o midi_send.o midiseq.o list.o iff.o fmio.o cm_500.o cm_32l.o c_vars.o amidilib.o tbl_stat.o minilzo.o memory.o amlog.o midi_rep.o
else
SRCS = main.c twisterm.c mt32.c midi_cmd.c midi_send.c midiseq.c list.c iff.c fmio.c cm_500.c cm_32l.c c_vars.c amidilib.c tbl_stat.c minilzo.c memory.c amlog.c mfp.c midi_rep.c
OBJECTS = main.o twisterm.o mt32.o midi_cmd.o midi_send.o midiseq.o list.o iff.o fmio.o cm_500.o cm_32l.o c_vars.o amidilib.o tbl_stat.o minilzo.o memory.o amlog.o midi_rep.o mfp.o
endif

ifeq ($(PORTABLE),1)
MIDITEST_SRCS = midiTest.c c_vars.c mt32.c midi_cmd.c midi_send.c midiseq.c fmio.c cm_500.c cm_32l.c amidilib.c tbl_stat.c list.c memory.c amlog.c
MIDITEST_OBJECTS = midiTest.o c_vars.o midi_cmd.o midi_send.o midiseq.o fmio.o cm_500.o cm_32l.o amidilib.o tbl_stat.o list.o memory.o amlog.o
else
MIDITEST_SRCS = midiTest.c c_vars.c mt32.c midi_cmd.c midi_send.c midiseq.c fmio.c cm_500.c cm_32l.c amidilib.c tbl_stat.c list.c memory.c amlog.c  mfp.c
MIDITEST_OBJECTS = midiTest.o c_vars.o midi_cmd.o midi_send.o midiseq.o fmio.o cm_500.o cm_32l.o amidilib.o tbl_stat.o list.o memory.o amlog.o  mfp.o
endif

ifeq ($(PORTABLE),1)
YM_TEST_SRCS = ymTest.c c_vars.c ym2149.c memory.c amlog.c
YM_TEST_OBJECTS = ymTest.o c_vars.o ym2149.o memory.o amlog.o
else
YM_TEST_SRCS = ymTest.c c_vars.c ym2149.c memory.c amlog.c mfp.c
YM_TEST_OBJECTS = ymTest.o c_vars.o ym2149.o memory.o amlog.o mfp.o
endif

ifeq ($(PORTABLE),1)
TIMING_TEST_SRCS = timTest.c ym2149.c c_vars.c mt32.c midi_cmd.c midi_send.c midiseq.c fmio.c cm_500.c cm_32l.c amidilib.c tbl_stat.c list.c memory.c amlog.c
TIMING_TEST_OBJECTS = timTest.o c_vars.o ym2149.o mt32.o midi_cmd.o midi_send.o midiseq.o fmio.o cm_500.o cm_32l.o amidilib.o tbl_stat.o list.o memory.o amlog.o
else
TIMING_TEST_SRCS = timTest.c ym2149.c c_vars.c mt32.c midi_cmd.c midi_send.c midiseq.c fmio.c cm_500.c cm_32l.c amidilib.c tbl_stat.c list.c memory.c amlog.c mfp.c
TIMING_TEST_OBJECTS = timTest.o c_vars.o ym2149.o mt32.o midi_cmd.o midi_send.o midiseq.o fmio.o cm_500.o cm_32l.o amidilib.o tbl_stat.o list.o memory.o amlog.o mfp.o
endif

ifeq ($(PORTABLE),1)
$(EXE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@ -lm 
else
$(EXE): $(OBJECTS) amidi.o int_rout.o ikbd_asm.o
	$(CC) $(LDFLAGS) $(OBJECTS) amidi.o int_rout.o ikbd.o -o $@ -lgem -lm 
endif
	echo "Setting AMIDI.TTP stack to: " $(STACK_SIZE)
#$(SET_STACK) $(EXE)
#	echo "Stripping symbols."
#	$(STRIP) $(EXE)
	echo "Copying AMIDI.TTP binary to emulator/shared directory."
	cp $(EXE) $(ST_HD_PATH)
	cp $(EXE) $(SHARED_DIR_PATH)
ifeq ($(PORTABLE),1)
$(YM_TEST_EXE): $(YM_TEST_OBJECTS) 
	$(CC) $(LDFLAGS) $(YM_TEST_OBJECTS) -o $@ -lm 
else
$(YM_TEST_EXE): $(YM_TEST_OBJECTS)  amidi.o 
	$(CC) $(LDFLAGS) $(YM_TEST_OBJECTS)  amidi.o -o $@ -lm 
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
$(TIMING_TEST_EXE): $(TIMING_TEST_OBJECTS) amidi.o ikbd_asm.o testReplay.o 
	$(CC) $(LDFLAGS) $(TIMING_TEST_OBJECTS) amidi.o ikbd.o testReplay.o -o $@ -lgem -lm 
endif
	echo "Stripping symbols"
	$(STRIP) $(TIMING_TEST_EXE)
	echo "Copying midi delta timing test program to emulator/shared directory."
	cp $(TIMING_TEST_EXE) $(ST_HD_PATH)
	cp $(TIMING_TEST_EXE) $(SHARED_DIR_PATH)

ifneq ($(PORTABLE),1)
amidi.o:	amidi.s
		$(ASM) amidi.s $(ASMFLAGS) -o amidi.o

int_rout.o:	int_rout.s
		$(ASM) int_rout.s $(ASMFLAGS) -o int_rout.o

testReplay.o:	testReplay.s
		$(ASM) testReplay.s $(ASMFLAGS) -o testReplay.o


ikbd_asm.o:	ikbd.S
		$(GAS) $(MACHINE) ikbd.S -o ikbd.o
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

