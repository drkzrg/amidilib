#!/bin/bash
# copies all needed files for building minimal, standalone nkt replay library.

echo Target directory cleanup
find ./nktlib/ -name "*.s" -type f|xargs rm -f
find ./nktlib/ -name "*.inc" -type f|xargs rm -f
find ./nktlib/ -name "*.h" -type f|xargs rm -f
find ./nktlib/ -name "*.c" -type f|xargs rm -f


echo Copying sources ..

if [ ! -d "./nktlib/src" ]; then
mkdir ./nktlib/src
fi

cp ../src/tos/gemdosio.c ./nktlib/src/
cp ../src/common/formats/nkt_rep.c  ./nktlib/src/
cp ../src/common/formats/nkt_rep_m68k.s ./nktlib/src/
cp ../src/common/roland/rol_ptch.c ./nktlib/src/
cp ../src/tos/common.s ./nktlib/src/
cp ../src/tos/memory.c ./nktlib/src/
cp ../src/common/memory/linalloc.c ./nktlib/src/
cp ../src/common/formats/midi2nkt.c ./nktlib/src/
cp ../src/common/formats/mus2midi.c ./nktlib/src/
cp ../src/common/core/c_vars.c ./nktlib/src/
cp ../src/common/core/amlog.c ./nktlib/src/
cp ../src/common/timing/miditim.c ./nktlib/src/
cp ../src/common/formats/midiInfo.c ./nktlib/src/
cp ../src/common/lzo/minilzo.c ./nktlib/src/

echo Copying headers..

if [ ! -d "./nktlib/include/" ]; then
mkdir ./nktlib/include/
fi

cp ../include/amlog.h ./nktlib/include/
cp ../include/common_m68k.inc ./nktlib/include/
cp ../include/vartypes.h ./nktlib/include/
cp ../include/dmus.h ./nktlib/include/
cp ../include/events.h ./nktlib/include/
cp ../include/gemdosio.h ./nktlib/include/
cp ../include/m68k_defs.inc ./nktlib/include/
cp ../include/mdevtype.h ./nktlib/include/
cp ../include/midi.h ./nktlib/include/
cp ../include/midi2nkt.h ./nktlib/include/
cp ../include/midi_cmd.h ./nktlib/include/
cp ../include/midi_send.h ./nktlib/include/
cp ../include/nkt.h ./nktlib/include/
cp ../include/nkt_util.h ./nktlib/include/
cp ../include/roland.h ./nktlib/include/
cp ../include/rol_ptch.h ./nktlib/include/
cp ../include/vendors.h ./nktlib/include/

if [ ! -d "./nktlib/include/core/" ]; then
mkdir ./nktlib/include/core/
fi

cp ../include/core/assert.h ./nktlib/include/core/
cp ../include/core/debug.h ./nktlib/include/core/


if [ ! -d "./nktlib/include/timing/" ]; then
mkdir ./nktlib/include/timing/
fi

cp ../include/timing/mfp.h ./nktlib/include/timing/
cp ../include/timing/mfp_m68k.inc ./nktlib/include/timing/
cp ../include/timing/miditim.h ./nktlib/include/timing/

if [ ! -d "./nktlib/include/memory/" ]; then
mkdir ./nktlib/include/memory/
fi

cp ../include/memory/linalloc.h ./nktlib/include/memory/
cp ../include/memory/memory.h ./nktlib/include/memory/
cp ../include/memory/endian.h ./nktlib/include/memory/

if [ ! -d "./nktlib/include/lzo/" ]; then
mkdir ./nktlib/include/lzo/
fi

cp ../include/lzo/lzoconf.h ./nktlib/include/lzo/
cp ../include/lzo/lzodefs.h ./nktlib/include/lzo/
cp ../include/lzo/minilzo.h ./nktlib/include/lzo/

echo Done...
