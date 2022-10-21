INCLUDE_PATH = include
INCLUDE_PATH_FLAGS = $(foreach i, $(INCLUDE_PATH),-I$i)

LIB_PATH = .
LIB_PATH_FLAGS = $(foreach i, $(LIB_PATH),-L$i)

LIBS = ncurses
LIBS_FLAGS = $(foreach i, $(LIBS),-l$i)

CFLAGS += -std=c++0x -pthread

DEBUG_OPTIONS = -g

SOURCES = src/ncui_screen.cc src/ncui_window.cc
OBJECTS=$(SOURCES:.cc=.o)

HEADERS = include/ncui_common.h include/ncui_types.h include/ncui_field_buffer.h include/ncui_screen.h include/ncui_window.h include/ncui.h

DEPENDENCIES = $(HEADERS)

all: tests/test_demo tests/test_focus tests/test_focus2 tests/test_focus3 tests/test_focus_mouse

$(OBJECTS): $(DEPENDENCIES)

.cc.o:
	g++ $(CFLAGS) $(DEBUG_OPTIONS) $(INCLUDE_PATH_FLAGS) $(LIB_PATH_FLAGS)  -c $< -o $@ $(LIBS_FLAGS)

tests/test_demo: $(OBJECTS) tests/test_demo.o
	g++ $(CFLAGS) $(DEBUG_OPTIONS) $(INCLUDE_PATH_FLAGS) $(LIB_PATH_FLAGS) $(OBJECTS) tests/test_demo.o -o $@ $(LIBS_FLAGS)

tests/test_focus: $(OBJECTS) tests/test_focus.o
	g++ $(CFLAGS) $(DEBUG_OPTIONS) $(INCLUDE_PATH_FLAGS) $(LIB_PATH_FLAGS) $(OBJECTS) tests/test_focus.o -o $@ $(LIBS_FLAGS)

tests/test_focus2: $(OBJECTS) tests/test_focus2.o
	g++ $(CFLAGS) $(DEBUG_OPTIONS) $(INCLUDE_PATH_FLAGS) $(LIB_PATH_FLAGS) $(OBJECTS) tests/test_focus2.o -o $@ $(LIBS_FLAGS)

tests/test_focus3: $(OBJECTS) tests/test_focus3.o
	g++ $(CFLAGS) $(DEBUG_OPTIONS) $(INCLUDE_PATH_FLAGS) $(LIB_PATH_FLAGS) $(OBJECTS) tests/test_focus3.o -o $@ $(LIBS_FLAGS)

tests/test_focus_mouse: $(OBJECTS) tests/test_focus_mouse.o
	g++ $(CFLAGS) $(DEBUG_OPTIONS) $(INCLUDE_PATH_FLAGS) $(LIB_PATH_FLAGS) $(OBJECTS) tests/test_focus_mouse.o -o $@ $(LIBS_FLAGS)

clean:
	rm -f $(OBJECTS) tests/test_demo.o tests/test_demo tests/test_focus.o tests/test_focus tests/test_focus2.o tests/test_focus2 tests/test_focus3.o tests/test_focus3 tests/test_focus_mouse.o tests/test_focus_mouse
