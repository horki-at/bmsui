cmake_minimum_required(VERSION 3.23)
project(bmsui)

set(CMAKE_CXX_FLAGS_DEBUG_INIT "-Wall")

# Compile vendor libraries.
add_subdirectory(vendor)

# Find bmsui source files.
file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc)

# Setup bmsui target.
add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)
target_link_libraries(${PROJECT_NAME} PRIVATE vendor)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_compile_definitions(${PROJECT_NAME}
  PUBLIC
  FONT_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets/iosevka-regular.ttf"
  ICON_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets/icon.png"
  POPULATE_BMS_SCRIPT="${CMAKE_CURRENT_SOURCE_DIR}/util/populate_bms.py"
  IMGUI_INI="${CMAKE_CURRENT_SOURCE_DIR}/assets/imgui.ini"
  _HAS_STD_BYTE=0
)

if (WIN32)
  target_compile_definitions(${PROJECT_NAME}
    PUBLIC
      PYTHON3_BIN="${CMAKE_CURRENT_SOURCE_DIR}/.venv/Scripts/python.exe"
      WINDOWS_BROKER="${CMAKE_CURRENT_SOURCE_DIR}/util/windows_broker.py"
  )
elseif (UNIX)
target_compile_definitions(${PROJECT_NAME}
    PUBLIC
      PYTHON3_BIN="${CMAKE_CURRENT_SOURCE_DIR}/.venv/bin/python3"
      START_COMM_SCRIPT="${CMAKE_CURRENT_SOURCE_DIR}/util/start_comm.sh"
  )
endif()