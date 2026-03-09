#!/bin/bash

VBMS="$1"
VAPP="$2"

PREFIX="$HOME/projects/bmsui"
socat -d -d PTY,link="$PREFIX/$VBMS",raw  \
            PTY,link="$PREFIX/$VAPP",raw > /dev/null
