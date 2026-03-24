add_library(glad STATIC)
target_sources(glad
  PRIVATE
    src/glad.c

  PUBLIC
    FILE_SET HEADERS
    BASE_DIRS
      include
    FILES
      include/glad/glad.h
      include/KHR/khrplatform.h
)
