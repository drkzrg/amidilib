#DEFINES = -DTOS -DNDEBUG -DGNU
#CXXFLAGS:=-g -O2 -m68020-60 -Wall -pedantic -ansi -fsigned-char -Wuninitialized -Wno-unknown-pragmas -Wshadow -Wimplicit
#CXXFLAGS+=-Wundef -Wreorder -Wwrite-strings -Wnon-virtual-dtor -Wno-multichar -fomit-frame-pointer -fno-exceptions -fno-rtti
#CXXFLAGS+=$(DEFINES)

INCLUDES = -I./ -I./INCLUDE -I/usr/m68k-atari-mint/include
CC = m68k-atari-mint-gcc
CFLAGS= -m68020-60 -Wall -pedantic -ansi
LDFLAGS=
EXE = amidi.tos

SRCS = MAIN.C TWISTERM.C MT32.C MIDI_CMD.C LIST.C IFF.C FMIO.C CM_500.C CM_32L.C C_VARS.C AMIDILIB.C
OBJECTS = $(SRCS:.c=.o)

$(EXE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(INCLUDES) $(OBJECTS) -o $@

all: $(SRCS) $(EXE)


.c.o:
	$(CC) $(CFLAGS) $< -o $@


clean:
	rm -rf *o amidi.tos

