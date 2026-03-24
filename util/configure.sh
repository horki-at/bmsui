#!/bin/bash

# initialize git submodules
git submodule init
git submodule update

cd vendor/imgui
git checkout docking
ln -s ../additional_cml/imgui/CMakeLists.txt

cd ../implot
ln -s ../additional_cml/implot/CMakeLists.txt

# Go into project root and setup python venv
cd ../../
uv venv ./.venv/
uv pip install --python ./.venv/bin/python3 -r requirements.txt
