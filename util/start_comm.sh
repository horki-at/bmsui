#!/bin/bash

VBMS="$1"
VAPP="$2"

socat -d -d PTY,link="${VBMS}",raw  \
            PTY,link="${VAPP}",raw > /dev/null
