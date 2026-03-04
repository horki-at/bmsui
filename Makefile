.RECIPEPREFIX=|

TARGET := bmsui

SOURCES := $(wildcard src/*.cc)
OBJECTS := $(SOURCES:.cc=.o)
DEPS := $(OBJECTS:.o=.o.dep)

vpath %.cc $(dir $(SOURCES))

all: build
build: $(TARGET)

clean:
| rm $(OBJECTS) $(DEPS) $(TARGET)

$(TARGET): $(OBJECTS)
| g++ -std=c++26 -Wall -Werror -pedantic -o $(TARGET) $(OBJECTS)

%.o: %.cc
| g++ -std=c++26 -Wall -Werror -pedantic -MMD -MP -MF $(@:=.dep) -c -o $@ $<

-include $(DEPS)
