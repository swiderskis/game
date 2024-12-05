# Makefile adapted from 
# https://stackoverflow.com/questions/30573481/how-to-write-a-makefile-with-separate-source-and-header-directories

# TODO make this cross-platform

RAYLIB_DIR := ext-lib/raylib/src
RAYLIB_CPP_DIR := ext-lib/raylib-cpp/include
RAYLIB := $(RAYLIB_DIR)/libraylib.a
RAYLIB_SO := $(RAYLIB_DIR)/libraylibdll.a

CPPFLAGS := -MMD -MP -isystem$(RAYLIB_DIR) -iquote$(RAYLIB_CPP_DIR)
CXXFLAGS := -O3 -Wall -Wextra -Wpedantic -Werror -std=c++23
LDFLAGS := -L$(RAYLIB_DIR)
LDLIBS := -lraylib -lopengl32 -lgdi32 -lwinmm
LDLIBS_SO := -lraylibdll -lopengl32 -lgdi32 -lwinmm

SRC_DIR := src
BIN_DIR := build
RELEASE_BIN_DIR = $(BIN_DIR)/release
DEBUG_BIN_DIR = $(BIN_DIR)/debug

BIN_NAME := game.exe
SO_NAME := game.dll
BIN := $(RELEASE_BIN_DIR)/$(BIN_NAME)
DEBUG_BIN := $(DEBUG_BIN_DIR)/$(BIN_NAME)
SO := $(DEBUG_BIN_DIR)/$(SO_NAME)
SRC := $(wildcard $(SRC_DIR)/*.cpp)
OBJ := $(SRC:$(SRC_DIR)/%.cpp=$(RELEASE_BIN_DIR)/%.o)
DEBUG_OBJ := $(SRC:$(SRC_DIR)/%.cpp=$(DEBUG_BIN_DIR)/%$(HR).o)

NDEBUG := -DNDEBUG
DEBUG := -DSO_NAME=\"$(SO)\"

.PHONY: all debug run release run_release so clean clean_raylib

all: debug

debug: $(DEBUG_BIN)

run: $(DEBUG_BIN)
	cp $(RAYLIB_DIR)/raylib.dll $(DEBUG_BIN_DIR)
	$(DEBUG_BIN)

release: $(BIN)

run_release: $(BIN)
	$(BIN)

so: $(SO)

clean:
	$(RM) -r $(BIN_DIR)

clean_raylib:
	$(RM) -r $(BIN_DIR)
	$(MAKE) clean_shell_sh -C $(RAYLIB_DIR)

$(BIN): $(RAYLIB) $(OBJ) | $(BIN_DIR)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(DEBUG_BIN): $(RAYLIB_SO) $(DEBUG_OBJ) | $(BIN_DIR) $(SO)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS_SO) -o $@

$(SO): $(RAYLIB_SO) $(DEBUG_OBJ) | $(BIN_DIR)
	$(CXX) -fPIC -shared $(LDFLAGS) $^ $(LDLIBS_SO) -o $@

$(RELEASE_BIN_DIR)/%.o: $(SRC_DIR)/%.cpp | $(RELEASE_BIN_DIR)
	$(CXX) $(CPPFLAGS) $(NDEBUG) $(CXXFLAGS) -c $< -o $@

$(DEBUG_BIN_DIR)/%.o: $(SRC_DIR)/%.cpp | $(DEBUG_BIN_DIR)
	$(CXX) $(CPPFLAGS) $(DEBUG) $(CXXFLAGS) -c $< -o $@

$(RAYLIB):
	$(MAKE) PLATFORM=PLATFORM_DESKTOP -C $(RAYLIB_DIR)

$(RAYLIB_SO):
	$(MAKE) PLATFORM=PLATFORM_DESKTOP RAYLIB_LIBTYPE=SHARED -C $(RAYLIB_DIR)

$(RELEASE_BIN_DIR): | $(BIN_DIR)
	mkdir $@

$(DEBUG_BIN_DIR): | $(BIN_DIR)
	mkdir $@

$(BIN_DIR):
	mkdir $@

-include $(OBJ:.o=.d)
