#DEFINES = -DTOS -DNDEBUG -DGNU
#CXXFLAGS:=-g -O2 -m68020-60 -Wall -pedantic -ansi -fsigned-char -Wuninitialized -Wno-unknown-pragmas -Wshadow -Wimplicit
#CXXFLAGS+=-Wundef -Wreorder -Wwrite-strings -Wnon-virtual-dtor -Wno-multichar -fomit-frame-pointer -fno-exceptions -fno-rtti
#CXXFLAGS+=$(DEFINES)

INCLUDES = -I./ -I./INCLUDE -I/usr/m68k-atari-mint/include
CC = m68k-atari-mint-gcc

# extra CFLAGS: -DDEBUG_BUILD -DDEBUG_FILE_OUTPUT -DDEBUG
CFLAGS += -g -m68000 -Wall -fsigned-char -pedantic -Wl,--traditional-format -DDEBUG_BUILD -DDEBUG_FILE_OUTPUT
LDFLAGS += -L/usr/m68k-atari-mint/lib
ASM = vasmm68k_mot
ASMFLAGS += -Faout -quiet -x -m68000 -spaces -showopt 
EXE = amidi.tos

SRCS = MAIN.C TWISTERM.C MT32.C MIDI_CMD.C MIDISEQ.C LIST.C IFF.C FMIO.C CM_500.C CM_32L.C C_VARS.C AMIDILIB.C TBL_STAT.C
OBJECTS = MAIN.O TWISTERM.O MT32.O MIDI_CMD.O MIDISEQ.O LIST.O IFF.O FMIO.O CM_500.O CM_32L.O C_VARS.O AMIDILIB.O TBL_STAT.O

$(EXE): $(OBJECTS) amidi.o int_routs.o
	$(CC) $(LDFLAGS) $(OBJECTS) int_rout.o amidi.o -o $@ -lgem -lm 
	
	
all: $(SRCS) $(EXE)
	
amidi.o:	AMIDI.S
		$(ASM) AMIDI.S $(ASMFLAGS) -o amidi.o

int_routs.o:	INT_ROUT.S
		$(ASM) INT_ROUT.S $(ASMFLAGS) -o int_rout.o

$(OBJECTS): 	%.O: %.C
		$(CC) -c $(CFLAGS) $< -o $@
clean:
	rm -rf *o *O amidi.tos

