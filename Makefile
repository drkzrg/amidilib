#DEFINES = -DTOS -DNDEBUG -DGNU
#CXXFLAGS:=-g -O2 -m68020-60 -Wall -pedantic -ansi -fsigned-char -Wuninitialized -Wno-unknown-pragmas -Wshadow -Wimplicit
#CXXFLAGS+=-Wundef -Wreorder -Wwrite-strings -Wnon-virtual-dtor -Wno-multichar -fomit-frame-pointer -fno-exceptions -fno-rtti
#CXXFLAGS+=$(DEFINES)

INCLUDES = -I./ -I./include -I/usr/m68k-atari-mint/include -I./include/lzo -I./include/ym2149
CC = m68k-atari-mint-gcc
GAS = m68k-atari-mint-as
STRIP = m68k-atari-mint-strip

# extra CFLAGS: -DDEBUG_BUILD -DDEBUG_FILE_OUTPUT -DDEBUG
CFLAGS +=-std=c99 -m68000 $(INCLUDES) -Wall -fsigned-char -fomit-frame-pointer -pedantic -DDEBUG_BUILD -DDEBUG_FILE_OUTPUT -DDEBUG_CONSOLE_OUTPUT
LDFLAGS += -L/usr/m68k-atari-mint/lib -Wl,--traditional-format 
ASM = vasmm68k_mot
ASMFLAGS += -Faout -quiet -x -m68000 -spaces -showopt 
EXE = amidi.tos

# ym2149 test output program 
YM_TEST_EXE = ym2149.tos

# midi test output program
MIDI_TEST_EXE = midiTest.tos

#timing test output program
TIMING_TEST_EXE = timTest.tos

#copies output binary to emulator folder ready to launch
ST_HD_PATH=$(HOME)/Dokumenty/Pulpit/tos/dyskST

SRCS = main.c twisterm.c mt32.c midi_cmd.c midiseq.c list.c iff.c fmio.c cm_500.c cm_32l.c c_vars.c amidilib.c tbl_stat.c minilzo.c ym2149.c
OBJECTS = main.o twisterm.o mt32.o midi_cmd.o midiseq.o list.o iff.o fmio.o cm_500.o cm_32l.o c_vars.o amidilib.o tbl_stat.o minilzo.o ym2149.o

MIDITEST_SRCS = midiTest.c c_vars.c mt32.c midi_cmd.c midiseq.c fmio.c cm_500.c cm_32l.c amidilib.c tbl_stat.c list.c 
MIDITEST_OBJECTS = midiTest.o c_vars.o midi_cmd.o midiseq.o fmio.o cm_500.o cm_32l.o amidilib.o tbl_stat.o list.o  

YM_TEST_SRCS = ymTest.c c_vars.c ym2149.c
YM_TEST_OBJECTS = ymTest.o c_vars.o ym2149.o

TIMING_TEST_SRCS = timTest.c ym2149.c c_vars.c mt32.c midi_cmd.c midiseq.c fmio.c cm_500.c cm_32l.c amidilib.c tbl_stat.c list.c
TIMING_TEST_OBJECTS = timTest.o c_vars.o ym2149.o mt32.o midi_cmd.o midiseq.o fmio.o cm_500.o cm_32l.o amidilib.o tbl_stat.o list.o


$(EXE): $(OBJECTS) amidi.o int_rout.o
	$(CC) $(LDFLAGS) $(OBJECTS) int_rout.o amidi.o -o $@ -lgem -lm 
	echo "Copying TOS binary to emulator directory."
	cp $(EXE) $(ST_HD_PATH)

$(YM_TEST_EXE): $(YM_TEST_OBJECTS) int_rout.o 
	$(CC) $(LDFLAGS) $(YM_TEST_OBJECTS) int_rout.o -o $@ -lm 
	echo "Stripping symbols"
	$(STRIP) $(YM_TEST_EXE)
	echo "Copying ym2149 test program to emulator directory."
	cp $(YM_TEST_EXE) $(ST_HD_PATH)


$(MIDI_TEST_EXE): $(MIDITEST_OBJECTS) int_rout.o ikbd_asm.o  
	$(CC) $(LDFLAGS) $(MIDITEST_OBJECTS) int_rout.o ikbd.o -o $@ -lm
	echo "Stripping symbols"
	$(STRIP) $(MIDI_TEST_EXE)
	echo "Copying midi output test program to emulator directory."
	cp $(MIDI_TEST_EXE) $(ST_HD_PATH)
	
$(TIMING_TEST_EXE): $(TIMING_TEST_OBJECTS) int_rout.o ikbd_asm.o testReplay.o 
	$(CC) $(LDFLAGS) $(TIMING_TEST_OBJECTS) int_rout.o ikbd.o testReplay.o -o $@ -lm 
	echo "Stripping symbols"
	$(STRIP) $(TIMING_TEST_EXE)
	echo "Copying midi delta timing test program to emulator directory."
	cp $(TIMING_TEST_EXE) $(ST_HD_PATH)


amidi.o:	amidi.s
		$(ASM) amidi.s $(ASMFLAGS) -o amidi.o

int_rout.o:	int_rout.s
		$(ASM) int_rout.s $(ASMFLAGS) -o int_rout.o

testReplay.o:	testReplay.s
		$(ASM) testReplay.s $(ASMFLAGS) -o testReplay.o


ikbd_asm.o:	ikbd.S
		$(GAS) ikbd.S -o ikbd.o



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

all: $(SRCS) $(EXE)
	
clean:
	rm -rf *o $(EXE)
	rm -rf *o $(YM_TEST_EXE)
	rm -rf *o $(MIDI_TEST_EXE)
	rm -rf *o $(TIMING_TEST_EXE)

.PHONY: default clean

