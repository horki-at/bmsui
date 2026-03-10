.RECIPEPREFIX=|

ifeq ($(shell test -w /usr/local/bin && echo yes),yes)
	PREFIX ?= /usr/local
else
	PREFIX ?= $(HOME)/.local
endif

.PHONY: build clean debug release install copy-files local-venv uninstall

CC  := gcc
CXX := g++ -std=c++26 -Wall -pedantic
CXXFLAGS :=

# Vendor: stb_image
STB_INCLUDE := -Ivendor/stb_image/

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
INCLUDE := -Isrc/ -Ivendor/imgui/ -Ivendor/implot/ -Ivendor/imgui/backends/ $(GLAD_INCLUDE) $(GLFW_INCLUDE) $(STB_INCLUDE)
LDFLAGS := -lGL $(GLFW_LIBRARY)

vpath %.cc $(dir $(SOURCES))

build: debug

clean:
| rm $(OBJECTS) $(DEPS) $(TARGET) $(GLAD_TARGET)
| rm -rf vendor/glfw/build/
| rm -rf .venv/

BINDIR := $(PREFIX)/bin
SCRDIR := $(PREFIX)/share/bmsui
TMPDIR := /tmp
LOCAL_SCRDIR := .
CXXFLAGS := -DSCRDIR=$(LOCAL_SCRDIR) -DTMPDIR=$(TMPDIR)

UV := "$(HOME)/.local/bin/uv"

copy-files:
| @mkdir -p $(SCRDIR)
| @cp -r ./assets/ $(SCRDIR)
| @cp -r ./util/ $(SCRDIR)
| @$(UV) venv $(SCRDIR)/.venv/
| @. $(SCRDIR)/.venv/bin/activate && $(UV) pip install -r requirements.txt
| @mkdir -p $(HOME)/.local/share/applications
| @cp ./assets/bmsui_template.desktop $(HOME)/.local/share/applications/bmsui.desktop
| @sed -i 's@PATH_TO_EXEC@$(BINDIR)/bmsui@' $(HOME)/.local/share/applications/bmsui.desktop
| @sed -i 's@PATH_TO_ICON@$(SCRDIR)/assets/icon.jpeg@' $(HOME)/.local/share/applications/bmsui.desktop

install: CXXFLAGS := $(filter-out -DSCRDIR=%,$(CXXFLAGS))
install: CXXFLAGS += -DSCRDIR=$(SCRDIR)
install: copy-files release
| @echo "Installing $(TARGET)... into $(BINDIR), and scripts into $(SCRDIR)"
| @install -d $(BINDIR)
| @install -m 755 $(TARGET) $(BINDIR)/$(TARGET)
| @echo "Finished."

uninstall:
| @echo "Uninstalling $(TARGET)..."
| @rm $(BINDIR)/$(TARGET)
| @rm -rf $(SCRDIR)
| @echo "Finished."

local-venv:
| $(UV) venv .venv/
| $(UV) pip install --python .venv/bin/python3 -r requirements.txt

debug: CXXFLAGS += -ggdb -DDEBUG -O1
debug: $(TARGET) local-venv

release: CXXFLAGS += -O3
release: $(TARGET) local-venv

$(TARGET): $(OBJECTS) $(GLAD_TARGET) | $(GLFW_TARGET)
| $(CXX) -o $@ $^ $(LDFLAGS)

%.o: %.cc
| $(CXX) $(CXXFLAGS) $(INCLUDE) -MMD -MP -MF $(@:=.dep) -c -o $@ $<
%.o: %.cpp
| $(CXX) $(CXXFLAGS) $(INCLUDE) -MMD -MP -MF $(@:=.dep) -c -o $@ $<

-include $(DEPS)
