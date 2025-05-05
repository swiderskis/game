# Makefile adapted from 
# https://stackoverflow.com/questions/30573481/how-to-write-a-makefile-with-separate-source-and-header-directories

BUILD_DIR := make-build
RELEASE_DIR := $(BUILD_DIR)/release
DEBUG_DIR := $(BUILD_DIR)/debug
SRC := $(wildcard src/*.cpp)
OBJ := $(SRC:src/%.cpp=$(RELEASE_DIR)/%.o)
DEBUG_OBJ := $(SRC:src/%.cpp=$(DEBUG_DIR)/%.o)
BIN_SUFFIX :=
SO_SUFFIX := .so
ifeq ($(OS), Windows_NT)
	BIN_SUFFIX = .exe
	SO_SUFFIX = .dll
endif
BIN := $(RELEASE_DIR)/game$(BIN_SUFFIX)
DEBUG_BIN := $(DEBUG_DIR)/game$(BIN_SUFFIX)
SO := $(DEBUG_DIR)/game$(SO_SUFFIX)

LIB := seb
LIB_DIRS := $(LIB:%=lib/%)
LIB_FILES := $(join $(LIB_DIRS), $(addsuffix .a,$(addprefix /lib,$(LIB))))
LIB_CPPFLAGS := $(LIB_DIRS:%=-iquote%/src)
LIB_LDFLAGS := $(LIB_DIRS:%=-L%)
LIB_LDLIBS := $(LIB:%=-l%)
LIB_CLEANS := $(LIB_DIRS:%=%/clean)

# shared library stub only required for Windows debug build
RAYLIB_DIR := ext-lib/raylib
RAYLIB_CPP_DIR := ext-lib/raylib-cpp
RAYLIB := $(RAYLIB_DIR)/src/libraylib.a
RAYLIB_SO := $(RAYLIB_DIR)/src/libraylib$(SO_SUFFIX)
RAYLIB_SO_STUB :=
ifeq ($(OS), Windows_NT)
	RAYLIB_SO = $(RAYLIB_DIR)/src/raylib$(SO_SUFFIX)
	RAYLIB_SO_STUB = $(RAYLIB_DIR)/src/libraylibdll.a
endif

CPPFLAGS := -MMD -MP $(LIB_CPPFLAGS) -isystem$(RAYLIB_DIR)/src -iquote$(RAYLIB_CPP_DIR)/include
CXXFLAGS := -O3 -Wall -Wextra -Wpedantic -Werror -std=c++23
LDFLAGS := -L$(RAYLIB_DIR)/src $(LIB_LDFLAGS)
LDLIBS := $(LIB_LDLIBS) -lraylib -lGL
LDLIBS_SO := $(LIB_LDLIBS) -lraylib -lGL
ifeq ($(OS), Windows_NT)
	LDLIBS = $(LIB_LDLIBS) -lraylib -lopengl32 -lgdi32 -lwinmm
	LDLIBS_SO = $(LIB_LDLIBS) -lraylibdll -lopengl32 -lgdi32 -lwinmm
endif

NDEBUG := -DNDEBUG
DEBUG := -DSO_NAME=\"$(SO)\"

# Windows binary searches the directory it sits in for shared libraries
# Linux binary doesn't, searches paths in LD_LIBRARY_PATH environment variable
DEBUG_RUN_SO_CMD :=
ifeq ($(OS), Windows_NT)
	DEBUG_RUN_SO_CMD = cp $(RAYLIB_SO) $(DEBUG_DIR)
else
	LD_LIBRARY_PATH_OLD := $(LD_LIBRARY_PATH)
	export LD_LIBRARY_PATH=$(PWD)/$(RAYLIB_DIR)/src:$(LD_LIBRARY_PATH_OLD)
endif

.PHONY: all debug run release run_release so clean $(LIB_CLEANS) clean_raylib $(LIB_FILES)

all: debug

debug: $(DEBUG_BIN)

run: $(DEBUG_BIN)
	$(DEBUG_RUN_SO_CMD)
	$(DEBUG_BIN)

release: $(BIN)

run_release: $(BIN)
	$(BIN)

so: $(SO)

clean: $(LIB_CLEANS)
	$(RM) -r $(BUILD_DIR)

$(LIB_CLEANS):
	@echo $(LIB_CLEANS)
	$(MAKE) clean -C$(dir $@)

clean_raylib: clean
	$(MAKE) clean_shell_sh -C$(RAYLIB_DIR)/src
	$(RM) $(RAYLIB_SO)
	$(RM) $(RAYLIB_SO_STUB)
	
# rm required on Linux build to prevent binary from linking to shared library
$(BIN): $(LIB_FILES) $(RAYLIB) $(OBJ) | $(BUILD_DIR)
	$(RM) $(RAYLIB_DIR)/src/libraylib$(SO_SUFFIX)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(DEBUG_BIN): $(LIB_FILES) $(RAYLIB_SO) $(DEBUG_OBJ) | $(BUILD_DIR) $(SO)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS_SO) -o $@

$(SO): $(RAYLIB_SO) $(DEBUG_OBJ) | $(BUILD_DIR)
	$(CXX) -fPIC -shared $(LDFLAGS) $^ $(LDLIBS_SO) -o $@

$(RELEASE_DIR)/%.o: src/%.cpp | $(RELEASE_DIR)
	$(CXX) $(CPPFLAGS) $(NDEBUG) $(CXXFLAGS) -c $< -o $@

$(DEBUG_DIR)/%.o: src/%.cpp | $(DEBUG_DIR)
	$(CXX) -fPIC $(CPPFLAGS) $(DEBUG) $(CXXFLAGS) -c $< -o $@

$(LIB_FILES):
	$(MAKE) RAYLIB_DIR=../../$(RAYLIB_DIR) RAYLIB_CPP_DIR=../../$(RAYLIB_CPP_DIR) -C$(dir $@)

$(RAYLIB):
	$(MAKE) PLATFORM=PLATFORM_DESKTOP -C$(RAYLIB_DIR)/src

$(RAYLIB_SO):
	$(MAKE) PLATFORM=PLATFORM_DESKTOP RAYLIB_LIBTYPE=SHARED -C$(RAYLIB_DIR)/src

$(RELEASE_DIR): | $(BUILD_DIR)
	mkdir $@

$(DEBUG_DIR): | $(BUILD_DIR)
	mkdir $@

$(BUILD_DIR):
	mkdir $@

-include $(OBJ:.o=.d)
-include $(DEBUG_OBJ:.o=.d)
