BUILD_DIR := build
DEBUG_DIR := $(BUILD_DIR)/debug
RELEASE_DIR := $(BUILD_DIR)/release
RAYLIB_DIR := ../../ext-lib/raylib
RAYLIB_CPP_DIR := ../../ext-lib/raylib-cpp
SEBLIB_DIR := ../seblib
LIB := libseb-engine.a
DEBUG_LIB := $(DEBUG_DIR)/$(LIB)
RELEASE_LIB := $(RELEASE_DIR)/$(LIB)
SRC := $(wildcard src/*.cpp)
DEBUG_OBJ := $(SRC:src/%.cpp=$(DEBUG_DIR)/%.o)
OBJ := $(SRC:src/%.cpp=$(RELEASE_DIR)/%.o)

CPPFLAGS := -MMD -MP -isystem$(RAYLIB_DIR)/src -iquote$(RAYLIB_CPP_DIR)/include -iquote$(SEBLIB_DIR)/src
CXXFLAGS := -O3 -Wall -Wextra -Wpedantic -Werror -std=c++23

NDEBUG := -DNDEBUG -DLOGLVL=2
DEBUG := -fPIC

.PHONY: all debug release clean

all: debug

debug: $(DEBUG_LIB)

release: $(RELEASE_LIB)

clean:
	$(RM) -r $(BUILD_DIR)

$(DEBUG_LIB): $(DEBUG_OBJ) | $(DEBUG_DIR)
	ar rcs $@ $^

$(RELEASE_LIB): $(OBJ) | $(RELEASE_DIR)
	ar rcs $@ $^

$(DEBUG_DIR)/%.o: src/%.cpp | $(DEBUG_DIR)
	$(CXX) $(CPPFLAGS) $(DEBUG) $(CXXFLAGS) -c $< -o $@

$(RELEASE_DIR)/%.o: src/%.cpp | $(RELEASE_DIR)
	$(CXX) $(CPPFLAGS) $(NDEBUG) $(CXXFLAGS) -c $< -o $@

$(DEBUG_DIR): | $(BUILD_DIR)
	mkdir $@

$(RELEASE_DIR): | $(BUILD_DIR)
	mkdir $@

$(BUILD_DIR):
	mkdir $@

-include $(DEBUG_OBJ:.o=.d)
-include $(OBJ:.o=.d)
