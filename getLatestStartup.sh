#!/bin/bash

# helper script
# downloads and installs custom gcc startup for brownelf / aout compilers

# Copyright 2007-2021 Pawel Goralski
#    
# This file is part of AMIDILIB.
# See license.txt for licensing information.
#

BUILD_ROOT=./

if [ -d "${BUILD_ROOT}/deps/atari-gcc-startup" ]; then
rm -rf ${BUILD_ROOT}/deps/atari-gcc-startup
fi

echo Update dependencies
wget https://bitbucket.org/nokturnal/atari-gcc-startup/downloads/atari-gcc-startup_latest.tgz -P ${BUILD_ROOT}
tar -zxvf ${BUILD_ROOT}/atari-gcc-startup_latest.tgz -C ${BUILD_ROOT}/deps
