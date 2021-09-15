#!/bin/bash

# helper script
# downloads and installs Libcmini for brownelf compilers

# Copyright 2007-2021 Pawel Goralski
#    
# This file is part of AMIDILIB.
# See license.txt for licensing information.
#

LIBCMINI_VER=0.54
BUILD_ROOT=./
BROWN_GCC_PREFIX=/opt/cross-mint/bin/m68k-ataribrown-elf-

if [ -d "${BUILD_ROOT}/deps/brownlibcmini" ]; then
rm -rf ${BUILD_ROOT}/deps/brownlibcmini
fi

if [ ! -f "${BUILD_ROOT}v${LIBCMINI_VER}.tar.gz" ]; then
echo "Archive doesn't exists. Downloading ..."
wget -q "https://github.com/freemint/libcmini/archive/refs/tags/v${LIBCMINI_VER}.tar.gz" -P ${BUILD_ROOT}
fi

mkdir ${BUILD_ROOT}/deps/brownlibcmini

tar -zxvf ${BUILD_ROOT}v${LIBCMINI_VER}.tar.gz -C ${BUILD_ROOT}deps/brownlibcmini --strip-components 1
cd ${BUILD_ROOT}/deps/brownlibcmini

echo patching

set -e			#stop on any error encountered
set -x         #echo all commands

fixregs()
{
sed -i -e "s/sp/%sp/gI" \
-e "s/a0/%a0/gI" \
-e "s/a1/%a1/gI" \
-e "s/a2/%a2/gI" \
-e "s/a3/%a3/gI" \
-e "s/a4/%a4/gI" \
-e "s/a5/%a5/gI" \
-e "s/a6/%a6/gI" \
-e "s/a7/%a7/gI" \
-e "s/d0/%d0/gI" \
-e "s/d1/%d1/gI" \
-e "s/d2/%d2/gI" \
-e "s/d3/%d3/gI" \
-e "s/d4/%d4/gI" \
-e "s/d5/%d5/gI" \
-e "s/d6/%d6/gI" \
-e "s/d7/%d7/gI" -i $1
}

fixregs sources/_infinitydf.S
fixregs sources/_normdf.S
fixregs sources/bcopy.S
fixregs sources/bzero.S
fixregs sources/crt0.S
fixregs sources/frexp.S
fixregs sources/ldexp.S
fixregs sources/memset.S
fixregs sources/minicrt0.S
fixregs sources/modf.S
fixregs sources/setstack.S
# This is wrong. gcc 6.2 will crash if a6 is added to the clobber list
# so we remove it. No idea what will happen though.
sed -i -e 's/, "%%a6"//gI' sources/setstack.S
sed -i -e "s/m68k-atari-mint-/${BROWN_GCC_PREFIX}/gI" tests/acctest/Makefile 
