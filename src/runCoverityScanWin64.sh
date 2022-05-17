#!/bin/bash

source userInfo.sh

#tool config (run once on coverity installation )
#./cov-analysis-win64/bin/cov-configure --gcc --comptype gcc --compiler /opt/cross-mint/bin/m68k-atari-mint-gcc-9.3.1
./cov-analysis-win64/bin/cov-configure -co /opt/cross-mint/bin/m68k-atari-mint-gcc-9.3.1.exe -- -m68020-40 -std=c99 -ffast-math

DATESTAMP=`date +%d_%m_%Y`
GITHEAD=`git rev-parse HEAD`

echo ###################### Clean build..
rm -rf ./cov-int
scons --sconstruct=SConstruct_020-40_release -c

echo ###################### Launching Coverity Scan
./cov-analysis-win64/bin/cov-build --dir cov-int bash -c "scons --sconstruct=SConstruct_020-40_release"

echo ####################### Packing output archive
tar czvf amidilib_gcc931_release_$DATESTAMP_$GITHEAD.tgz cov-int

echo ####################### Upload to Coverity
curl --form token=$coverityToken \
  --form email=$coverityEmail \
  --form file=@amidilib_gcc931_release_$DATESTAMP_$GITHEAD.tgz \
  --form version=$GITHEAD \
  --form description="AMIDILIB gcc 9.3.1 release" \
  $coverityUrl