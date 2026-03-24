:: initialize git submodules
git submodule init
git submodule update

cd vendor/imgui
git checkout docking
mklink "CMakeLists.txt" "../additional_cml/imgui/CMakeLists.txt"

cd ../implot
mklink "CMakeLists.txt" "../additional_cml/implot/CMakeLists.txt"

:: Go into project root and setup python venv
cd ../../
uv venv ./.venv/
uv pip install --python ./.venv/Scripts/python.exe -r requirements.txt
