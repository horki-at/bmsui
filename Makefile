.RECIPEPREFIX=|

.PHONY: build clean

CC  := gcc
CXX := g++ -std=c++26 -Wall -pedantic

# Vendor: glad 
GLAD_TARGET  := vendor/glad/src/glad.o
GLAD_INCLUDE := -Ivendor/glad/include/
GLAD_SOURCE  := vendor/glad/src/glad.c

$(GLAD_TARGET): $(GLAD_SOURCE)
| $(CC) $(GLAD_INCLUDE) -c -o $@ $^

# Vendor: glfw
GLFW_INCLUDE := -Ivendor/glfw/include/
GLFW_TARGET := vendor/glfw/build/src/libglfw3.a
GLFW_LIBRARY := -Lvendor/glfw/build/src -lglfw3

$(GLFW_TARGET):
| mkdir -p vendor/glfw/build/
| cmake -Svendor/glfw/ -Bvendor/glfw/build/
| cmake --build vendor/glfw/build/

# The main application settings. NOTE: ImGui is build with the main application
# as advised by the install tutorial.
TARGET := bmsui

SOURCES := $(wildcard src/*.cc vendor/imgui/*.cpp vendor/implot/*.cpp)    \
           vendor/imgui/backends/imgui_impl_glfw.cpp                      \
           vendor/imgui/backends/imgui_impl_opengl3.cpp
OBJECTS := $(patsubst %.cc,%.o,$(patsubst %.cpp,%.o,$(SOURCES)))
DEPS := $(OBJECTS:.o=.o.dep)
INCLUDE := -Isrc/ -Ivendor/imgui/ -Ivendor/implot/ -Ivendor/imgui/backends/ $(GLAD_INCLUDE) $(GLFW_INCLUDE) 
LDFLAGS := -lGL $(GLFW_LIBRARY)

vpath %.cc $(dir $(SOURCES))

build: $(TARGET)

clean:
| rm $(OBJECTS) $(DEPS) $(TARGET) $(GLAD_TARGET) 
| rm -rf vendor/glfw/build/

$(TARGET): $(OBJECTS) $(GLAD_TARGET) | $(GLFW_TARGET)
| $(CXX) -o $@ $^ $(LDFLAGS)

%.o: %.cc
| $(CXX) $(INCLUDE) -MMD -MP -MF $(@:=.dep) -c -o $@ $<
%.o: %.cpp
| $(CXX) $(INCLUDE) -MMD -MP -MF $(@:=.dep) -c -o $@ $<

-include $(DEPS)
