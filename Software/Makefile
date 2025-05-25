# ===================== CONFIG =====================
PROJECT_NAME ?= main
RAYLIB_PATH ?= C:/raylib/raylib
BUILD_MODE ?= RELEASE

# Determine OS
ifeq ($(OS),Windows_NT)
    PLATFORM = WINDOWS
    EXT = .exe
else
    PLATFORM = LINUX
    EXT =
endif

# ===================== FILES =====================
# Compatible recursive wildcard: Windows fallback
ifeq ($(PLATFORM),WINDOWS)
    SRC := $(wildcard src/*.cpp) $(wildcard src/*/*.cpp) $(wildcard src/*/*/*.cpp)
else
    SRC := $(shell find src -type f \( -name "*.cpp" -o -name "*.c" \))
endif

# ===================== COMPILER =====================
CC = g++
CFLAGS = -Wall -std=c++14 -D_DEFAULT_SOURCE -Wno-missing-braces

ifeq ($(BUILD_MODE),DEBUG)
    CFLAGS += -g -O0
else
    CFLAGS += -s -O1
endif

ifeq ($(PLATFORM),WINDOWS)
    CFLAGS += $(RAYLIB_PATH)/src/raylib.rc.data
endif

# ===================== INCLUDE & LIBS =====================
# Cross-platform include path expansion
ifeq ($(PLATFORM),WINDOWS)
    SRC_DIRS := $(wildcard src/*) $(wildcard src/*/*)
else
    SRC_DIRS := $(shell find src -type d)
endif

INCLUDE_PATHS := $(foreach dir,$(SRC_DIRS),-I$(dir))
INCLUDE_PATHS += -Isrc -I$(RAYLIB_PATH)/src -I$(RAYLIB_PATH)/src/external

LDFLAGS = -L$(RAYLIB_PATH)/src

ifeq ($(PLATFORM),WINDOWS)
    LDLIBS = -lraylib -lopengl32 -lgdi32 -lwinmm
else
    LDLIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
endif

# ===================== TARGETS =====================
all: $(PROJECT_NAME)

$(PROJECT_NAME):
	$(CC) -o $(PROJECT_NAME)$(EXT) $(SRC) $(CFLAGS) $(INCLUDE_PATHS) $(LDFLAGS) $(LDLIBS) -DPLATFORM_DESKTOP

clean:
ifeq ($(PLATFORM),WINDOWS)
	del /Q *.exe *.o 2>nul || exit 0
else
	rm -f $(PROJECT_NAME)$(EXT) *.o
endif
