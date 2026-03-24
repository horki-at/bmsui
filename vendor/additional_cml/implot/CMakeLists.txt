add_library(implot STATIC)
target_link_libraries(implot
  PUBLIC
    imgui
)
target_sources(implot
  PRIVATE
    implot.cpp
    implot_demo.cpp
    implot_items.cpp

  PUBLIC
    FILE_SET HEADERS
    FILES
      implot.h
      implot_internal.h
)
