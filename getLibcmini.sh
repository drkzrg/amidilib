#!/bin/bash

# helper script
# downloads and installs Libcmini

# Copyright 2007-2021 Pawel Goralski
#    
# This file is part of AMIDILIB.
# See license.txt for licensing information.
#

LIBCMINI_VER=0.54
BUILD_ROOT=./

if [ -d "${BUILD_ROOT}/deps/libcmini" ]; then
rm -rf ${BUILD_ROOT}/deps/libcmini
fi

mkdir ${BUILD_ROOT}/deps/libcmini

wget -q "https://github.com/freemint/libcmini/archive/refs/tags/v${LIBCMINI_VER}.tar.gz" -P ${BUILD_ROOT}
tar -zxvf ${BUILD_ROOT}v${LIBCMINI_VER}.tar.gz -C ${BUILD_ROOT}deps/libcmini --strip-components 1
