#DEFINES = -DTOS -DNDEBUG -DGNU
#CXXFLAGS:=-g -O2 -m68020-60 -Wall -pedantic -ansi -fsigned-char -Wuninitialized -Wno-unknown-pragmas -Wshadow -Wimplicit
#CXXFLAGS+=-Wundef -Wreorder -Wwrite-strings -Wnon-virtual-dtor -Wno-multichar -fomit-frame-pointer -fno-exceptions -fno-rtti
#CXXFLAGS+=$(DEFINES)

INCLUDES = -I./ -I./INCLUDE -I/usr/m68k-atari-mint/include
CC = m68k-atari-mint-gcc
CFLAGS += -m68020-60 -Wall -pedantic -Wl,--traditional-format
LDFLAGS += -L/usr/m68k-atari-mint/lib
EXE = amidi.tos

SRCS = MAIN.C TWISTERM.C MT32.C MIDI_CMD.C MIDISEQ.C LIST.C IFF.C FMIO.C CM_500.C CM_32L.C C_VARS.C AMIDILIB.C TBL_STAT.C
OBJECTS = $(SRCS:.c=.o)

$(EXE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(INCLUDES) $(OBJECTS) $(CFLAGS) -o $@ -lgem

all: $(SRCS) $(EXE)
	cp $(EXE) ~/Pulpit/aranym/c


.c.o:
	$(CC) $(CFLAGS) $< -o $@


clean:
	rm -rf *o amidi.tos

