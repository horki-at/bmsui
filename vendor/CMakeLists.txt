# Compile each individual vendor library
add_subdirectory(glad)
add_subdirectory(glfw)
add_subdirectory(stb)
add_subdirectory(imgui)
add_subdirectory(implot)

add_library(vendor INTERFACE)
target_link_libraries(vendor
  INTERFACE
  glad stb glfw imgui implot
)
