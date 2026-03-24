cmake_minimum_required(VERSION 3.23)
project(bmsui)

# Compile vendor libraries.
add_subdirectory(vendor)

# Find bmsui source files.
file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc)

# Setup bmsui target.
add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)
target_link_libraries(${PROJECT_NAME} PRIVATE vendor)

target_compile_definitions(${PROJECT_NAME}
  PUBLIC
  FONT_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets/iosevka-regular.ttf"
  ICON_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets/icon.jpeg"
  START_COMM_SCRIPT="${CMAKE_CURRENT_SOURCE_DIR}/util/start_comm.sh"
  POPULATE_BMS_SCRIPT="${CMAKE_CURRENT_SOURCE_DIR}/util/populate_bms.py"
  PYTHON3_BIN="${CMAKE_CURRENT_SOURCE_DIR}/.venv/bin/python3"
  TTYVBMS="/tmp/ttyVBMS"
  TTYVAPP="/tmp/ttyVAPP"
  IMGUI_INI="${CMAKE_CURRENT_SOURCE_DIR}/assets/imgui.ini"
)
