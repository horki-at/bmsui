#!/bin/bash

git submodule init
git submodule update
cd vendor/imgui && git checkout docking
