#!/bin/bash

# TODO: explain what this script does.

PREFIX="$HOME/projects/bmsui"
socat -d -d PTY,link="$PREFIX/ttyVBMS",raw  \
            PTY,link="$PREFIX/ttyVAPP",raw
