INCLUDE_PATH = include
LIB_PATH = .

LIBS = ncurses

CFLAGS += -std=c++0x -pthread

DEBUG_OPTIONS = -g

SOURCES = src/ncui_screen.cc src/ncui_window.cc demo.cc
OBJECTS=$(SOURCES:.cc=.o)

HEADERS = include/ncui_common.h include/ncui_types.h include/ncui_field_buffer.h include/ncui_screen.h include/ncui_window.h include/ncui.h

DEPENDENCIES = $(HEADERS)

all: demo

$(OBJECTS): $(DEPENDENCIES)

.cc.o:
	g++ $(CFLAGS) $(DEBUG_OPTIONS) -I$(INCLUDE_PATH) -L$(LIB_PATH)  -c $< -o $@ -l$(LIBS)

demo: $(OBJECTS)
	g++ $(CFLAGS) $(DEBUG_OPTIONS) -I$(INCLUDE_PATH) -L$(LIB_PATH) $(OBJECTS) -o $@ -l$(LIBS)

clean:
	rm -f $(OBJECTS) demo
