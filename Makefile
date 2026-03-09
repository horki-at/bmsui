.RECIPEPREFIX=|

.PHONY: build clean debug release install

CC  := gcc
CXX := g++ -std=c++26 -Wall -pedantic
CXXFLAGS :=

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

build: debug

clean:
| rm $(OBJECTS) $(DEPS) $(TARGET) $(GLAD_TARGET) 
| rm -rf vendor/glfw/build/

BINDIR := $(PREFIX)/bin
SCRDIR := $(PREFIX)/share/bmsui
LOCAL_SCRDIR := .
CXXFLAGS := -DSCRDIR=$(LOCAL_SCRDIR)

UV := "$(HOME)/.local/bin/uv"

install: CXXFLAGS := $(filter-out -DSCRDIR=%,$(CXXFLAGS))
install: CXXFLAGS += -DSCRDIR=$(SCRDIR)
install: release
| @echo "Installing $(TARGET)... into $(BINDIR), and scripts into $(SCRDIR)"
| @install -d $(BINDIR)
| @install -m 755 $(TARGET) $(BINDIR)/$(TARGET)
| @mkdir -p $(SCRDIR)
| @cp -r ./assets/ $(SCRDIR)
| @cp -r ./util/ $(SCRDIR)
| @$(UV) venv $(SCRDIR)/.venv/
| @. $(SCRDIR)/.venv/bin/activate && $(UV) pip install -r requirements.txt
| @echo "Finished."

uninstall:
| @echo "Uninstalling $(TARGET)..."
| @rm $(BINDIR)/$(TARGET)
| @rm -rf $(SCRDIR)
| @echo "Finished."

debug: CXXFLAGS += -ggdb -DDEBUG -O1
debug: $(TARGET)

release: CXXFLAGS += -O3
release: $(TARGET)

$(TARGET): $(OBJECTS) $(GLAD_TARGET) | $(GLFW_TARGET)
| $(CXX) -o $@ $^ $(LDFLAGS)

%.o: %.cc
| $(CXX) $(CXXFLAGS) $(INCLUDE) -MMD -MP -MF $(@:=.dep) -c -o $@ $<
%.o: %.cpp
| $(CXX) $(CXXFLAGS) $(INCLUDE) -MMD -MP -MF $(@:=.dep) -c -o $@ $<

-include $(DEPS)
