#!/bin/bash

# helper script
# downloads Coverity Scan suite Win64

# Copyright 2007-2022 Pawel Goralski
#    
# This file is part of AMIDILIB.
# See license.txt for licensing information.
#

BUILD_ROOT=.
COVERITY_VERSION=2021.12

if ! [ -f "${BUILD_ROOT}/cov-analysis-win64-${COVERITY_VERSION}.zip" ]; then
	echo "Getting coverity scan suite ..."
	wget -nc https://nokturnal.pl/downloads/atari/toolchains/cov-analysis-win64-${COVERITY_VERSION}.zip -P ${BUILD_ROOT}/
else
	echo "Coverity already downloaded!"
fi

if [ -f "${BUILD_ROOT}/cov-analysis-win64-${COVERITY_VERSION}.zip" ]; then
	echo Unpacking ...
	unzip -qq -o ${BUILD_ROOT}/cov-analysis-win64-${COVERITY_VERSION}.zip -d ${BUILD_ROOT}/src 
	mv ${BUILD_ROOT}/src/cov-analysis-win64-${COVERITY_VERSION}.1/ ${BUILD_ROOT}/src/cov-analysis-win64/
else
	echo "Coverity archive doesn't exist!"
fi

echod "Done!"
