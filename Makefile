# Makefile adapted from 
# https://stackoverflow.com/questions/30573481/how-to-write-a-makefile-with-separate-source-and-header-directories

RAYLIB_DIR := ext-lib/raylib/src
RAYLIB_CPP_DIR := ext-lib/raylib-cpp/include

CPPFLAGS := -MMD -MP -isystem$(RAYLIB_DIR) -iquote$(RAYLIB_CPP_DIR)
CXXFLAGS := -O3 -Wall -Wextra -std=c++20
LDFLAGS := -L$(RAYLIB_DIR)
LDLIBS := -lraylib -lopengl32 -lgdi32 -lwinmm
BIN_NAME := game.exe

SRC_DIR := src
BIN_DIR := build

BIN := $(BIN_DIR)/$(BIN_NAME)
SRC := $(wildcard $(SRC_DIR)/*.cpp)
OBJ := $(SRC:$(SRC_DIR)/%.cpp=$(BIN_DIR)/%.o)
RAYLIB := $(RAYLIB_DIR)/libraylib.a

.PHONY: all run clean clean_raylib

all: $(BIN)

run: $(BIN)
	$(BIN)

clean:
	$(RM) -r $(BIN_DIR)

clean_raylib:
	$(RM) -r $(BIN_DIR)
	$(MAKE) clean_shell_sh -C $(RAYLIB_DIR)

$(BIN): $(RAYLIB) $(OBJ) | $(BIN_DIR)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(BIN_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BIN_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(RAYLIB):
	$(MAKE) PLATFORM=PLATFORM_DESKTOP -C $(RAYLIB_DIR)

$(BIN_DIR):
	mkdir $@

-include $(OBJ:.o=.d)
