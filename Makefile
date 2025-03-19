# Makefile adapted from 
# https://stackoverflow.com/questions/30573481/how-to-write-a-makefile-with-separate-source-and-header-directories

SO_SUFFIX := .so
ifeq ($(OS), Windows_NT)
	SO_SUFFIX = .dll
endif

# shared library stub only required for Windows debug build
RAYLIB_DIR := ext-lib/raylib/src
RAYLIB_CPP_DIR := ext-lib/raylib-cpp/include
RAYLIB := $(RAYLIB_DIR)/libraylib.a
RAYLIB_SO := $(RAYLIB_DIR)/libraylib$(SO_SUFFIX)
RAYLIB_SO_STUB :=
ifeq ($(OS), Windows_NT)
	RAYLIB_SO = $(RAYLIB_DIR)/raylib$(SO_SUFFIX)
	RAYLIB_SO_STUB = $(RAYLIB_DIR)/libraylibdll.a
endif

CPPFLAGS := -MMD -MP -isystem$(RAYLIB_DIR) -iquote$(RAYLIB_CPP_DIR)
CXXFLAGS := -O3 -Wall -Wextra -Wpedantic -Werror -std=c++23
LDFLAGS := -L$(RAYLIB_DIR)
LDLIBS := -lraylib -lGL
LDLIBS_SO := -lraylib -lGL
ifeq ($(OS), Windows_NT)
	LDLIBS = -lraylib -lopengl32 -lgdi32 -lwinmm
	LDLIBS_SO = -lraylibdll -lopengl32 -lgdi32 -lwinmm
endif

SRC_DIR := src
BIN_DIR := make-build
RELEASE_BIN_DIR = $(BIN_DIR)/release
DEBUG_BIN_DIR = $(BIN_DIR)/debug

BIN_SUFFIX :=
ifeq ($(OS), Windows_NT)
	BIN_SUFFIX = .exe
endif
BIN_NAME := game$(BIN_SUFFIX)
SO_NAME := game$(SO_SUFFIX)
BIN := $(RELEASE_BIN_DIR)/$(BIN_NAME)
DEBUG_BIN := $(DEBUG_BIN_DIR)/$(BIN_NAME)
SO := $(DEBUG_BIN_DIR)/$(SO_NAME)
SRC := $(wildcard $(SRC_DIR)/*.cpp)
OBJ := $(SRC:$(SRC_DIR)/%.cpp=$(RELEASE_BIN_DIR)/%.o)
DEBUG_OBJ := $(SRC:$(SRC_DIR)/%.cpp=$(DEBUG_BIN_DIR)/%$(HR).o)

NDEBUG := -DNDEBUG
DEBUG := -DSO_NAME=\"$(SO)\"

# Windows binary searches the directory it sits in for shared libraries
# Linux binary doesn't, searches paths in LD_LIBRARY_PATH environment variable
DEBUG_RUN_SO_CMD :=
ifeq ($(OS), Windows_NT)
	DEBUG_RUN_SO_CMD = cp $(RAYLIB_SO) $(DEBUG_BIN_DIR)
else
	LD_LIBRARY_PATH_OLD := $(LD_LIBRARY_PATH)
	export LD_LIBRARY_PATH=$(PWD)/$(RAYLIB_DIR):$(LD_LIBRARY_PATH_OLD)
endif

.PHONY: all debug run release run_release so clean clean_raylib

all: debug

debug: $(DEBUG_BIN)

run: $(DEBUG_BIN)
	$(DEBUG_RUN_SO_CMD)
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
	$(RM) $(RAYLIB_SO)
	$(RM) $(RAYLIB_SO_STUB)
	
# rm required on Linux build to prevent binary from linking to shared library
$(BIN): $(RAYLIB) $(OBJ) | $(BIN_DIR)
	$(RM) $(RAYLIB_DIR)/libraylib$(SO_SUFFIX)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(DEBUG_BIN): $(RAYLIB_SO) $(DEBUG_OBJ) | $(BIN_DIR) $(SO)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS_SO) -o $@

$(SO): $(RAYLIB_SO) $(DEBUG_OBJ) | $(BIN_DIR)
	$(CXX) -fPIC -shared $(LDFLAGS) $^ $(LDLIBS_SO) -o $@

$(RELEASE_BIN_DIR)/%.o: $(SRC_DIR)/%.cpp | $(RELEASE_BIN_DIR)
	$(CXX) $(CPPFLAGS) $(NDEBUG) $(CXXFLAGS) -c $< -o $@

$(DEBUG_BIN_DIR)/%.o: $(SRC_DIR)/%.cpp | $(DEBUG_BIN_DIR)
	$(CXX) -fPIC $(CPPFLAGS) $(DEBUG) $(CXXFLAGS) -c $< -o $@

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
-include $(DEBUG_OBJ:.o=.d)
