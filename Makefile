###############################################################################
# RetroMania Arcade - Professional Game Engine Makefile
###############################################################################

CXX       := g++
CXXFLAGS  := -std=c++17 -O2 -Wall -Wextra -Wpedantic \
             -Wno-unused-parameter -Wno-sign-compare
LDFLAGS   := -lX11 -lXext
INCLUDES  := -I. -I./src

SRCDIR    := src
LIBDIR    := lib
BINDIR    := bin
TARGET    := $(BINDIR)/GameAracedeApp

# Source directories
CMPDIR    := $(SRCDIR)/cmp
MANDIR    := $(SRCDIR)/man
SYSDIR    := $(SRCDIR)/sys
UTILDIR   := $(SRCDIR)/util

# All source files
SOURCES   := $(SRCDIR)/main.cpp \
             $(CMPDIR)/entity.cpp \
             $(MANDIR)/entitymanager.cpp \
             $(MANDIR)/componentstoreage.cpp \
             $(SYSDIR)/rendersystem.cpp \
             $(SYSDIR)/physics.cpp \
             $(SYSDIR)/input.cpp \
             $(SYSDIR)/collision.cpp \
             $(SYSDIR)/audio.cpp \
             $(LIBDIR)/tinyPTC/src/xlib.c \
             $(LIBDIR)/tinyPTC/src/convert.c \
             $(LIBDIR)/picoPNG/src/picopng.cpp

OBJECTS   := $(SOURCES:.cpp=.o)
OBJECTS   := $(OBJECTS:.c=.o)

# Default target
all: $(BINDIR) $(TARGET)

$(BINDIR):
	mkdir -p $(BINDIR)

# Link
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^ $(LDFLAGS)
	@echo ""
	@echo "========================================"
	@echo "   Arcade — Build Complete!"
	@echo "  Run: ./bin/GameAracedeApp"
	@echo "========================================"

# Compile C++ sources
.cpp.o:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Compile C sources
.c.o:
	$(CC) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Clean
clean:
	rm -f $(OBJECTS) $(TARGET)
	rm -rf $(BINDIR)

# Run
run: all
	./$(TARGET)

# Debug build
debug: CXXFLAGS += -g -DDEBUG -O0
debug: clean all

.PHONY: all clean run debug
