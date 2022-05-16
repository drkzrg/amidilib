#!/bin/bash

source userInfo.sh

#tool config (run once on coverity installation )
#$coverityToolsPath/cov-configure --comptype gcc --compiler /opt/cross-mint/bin/m68k-ataribrown-elf-gcc
DATESTAMP=`date +%d_%m_%Y`
GITHEAD=`git rev-parse HEAD`

echo ###################### Clean build..
rm -rf ./cov-int
scons --sconstruct=SConstruct_release -c

echo ###################### Launching Coverity Scan
~/cov-analysis/bin/cov-build --dir cov-int bash -c "scons --sconstruct=SConstruct_release"

echo ####################### Packing output archive
tar czvf amidilib_gcc434_release_$DATESTAMP_$GITHEAD.tgz cov-int

echo ####################### Upload to Coverity
curl --form token=$coverityToken \
  --form email=$coverityEmail \
  --form file=@amidilib_gcc434_release_$DATESTAMP_$GITHEAD.tgz \
  --form version=$GITHEAD \
  --form description="Amidilib gcc434 release" \
  $coverityUrl