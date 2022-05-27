#!/bin/bash

CURDIR=$(dirname "$0")

REMOTE_MACHINE='6.6.6.15'
REMOTE_PATH='/e/adebug'
PRG='E:/ADEBUG/RELEASE/MIDIPLAY./TTP'
PARAM='U8_ETHPL.MID'

URL=http://$REMOTE_MACHINE$REMOTE_PATH/ADEBUG.TTP?run="$PRG $PARAM"

echo Launching debugger with $PRG and parameters: $PARAM
#curl -0 $URL
${CURDIR}/uip/uip-run.py $REMOTE_MACHINE$REMOTE_PATH/ADEBUG.TTP $PRG $PARAM