add_library(stb INTERFACE)
target_sources(stb
  INTERFACE
    FILE_SET HEADERS
    FILES
      stb_image.h
)
