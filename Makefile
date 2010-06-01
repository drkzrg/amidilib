#DEFINES = -DTOS -DNDEBUG -DGNU
#CXXFLAGS:=-g -O2 -m68020-60 -Wall -pedantic -ansi -fsigned-char -Wuninitialized -Wno-unknown-pragmas -Wshadow -Wimplicit
#CXXFLAGS+=-Wundef -Wreorder -Wwrite-strings -Wnon-virtual-dtor -Wno-multichar -fomit-frame-pointer -fno-exceptions -fno-rtti
#CXXFLAGS+=$(DEFINES)

INCLUDES = -I./ -I./INCLUDE -I/usr/m68k-atari-mint/include
CC = m68k-atari-mint-gcc

# extra CFLAGS: -DDEBUG_BUILD -DDEBUG_FILE_OUTPUT -DDEBUG
CFLAGS += -g -std=c99 -m68000 -Wall -fsigned-char -pedantic -Wall -DDEBUG_BUILD -DDEBUG_FILE_OUTPUT -DDEBUG_CONSOLE_OUTPUT
LDFLAGS += -L/usr/m68k-atari-mint/lib -Wl,--traditional-format 
ASM = vasmm68k_mot
ASMFLAGS += -Faout -quiet -x -m68000 -spaces -showopt 
EXE = amidi.tos

SRCS = main.c twisterm.c mt32.c midi_cmd.c midiseq.c list.c iff.c fmio.c cm_500.c cm_32l.c c_vars.c amidilib.c tbl_stat.c
OBJECTS = main.o twisterm.o mt32.o midi_cmd.o midiseq.o list.o iff.o fmio.o cm_500.o cm_32l.o c_vars.o amidilib.o tbl_stat.o

$(EXE): $(OBJECTS) amidi.o int_routs.o
	$(CC) $(LDFLAGS) $(OBJECTS) int_rout.o amidi.o -o $@ -lgem -lm 
	
	
all: $(SRCS) $(EXE)
	
amidi.o:	amidi.s
		$(ASM) amidi.s $(ASMFLAGS) -o amidi.o

int_routs.o:	int_rout.s
		$(ASM) int_rout.s $(ASMFLAGS) -o int_rout.o

$(OBJECTS): 	%.o: %.c
		$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm -rf *o amidi.tos

