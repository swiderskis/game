BIN := $(BUILD_DIR)/$(BIN)
SO := $(BUILD_DIR)/game$(SO_EXT)
ifeq ($(BUILD_TYPE), RELEASE)
	SO :=
endif
SRC := $(wildcard src/*.cpp)
OBJ := $(SRC:src/%.cpp=$(BUILD_DIR)/%.o)

CPPFLAGS := -MMD -MP
CPPFLAGS += $(addsuffix /src, $(addprefix -iquotelib/, $(LIBS)))
CPPFLAGS += -isystem$(RAYLIB_DIR)/src
CPPFLAGS += -iquote$(RAYLIB_CPP_DIR)/include
CXXFLAGS := -std=c++23
ifeq ($(BUILD_TYPE), RELEASE)
	CPPFLAGS += -DNDEBUG -DLOGLVL=2
	CXXFLAGS += -O3
else
	CPPFLAGS += -fPIC -DSO_NAME=\"$(SO)\"
	CXXFLAGS += -O2 -Wall -Wextra -Wpedantic -Werror
endif

LDFLAGS := $(LIBS:%=-Llib/%/$(BUILD_DIR))
LDFLAGS += -L$(RAYLIB_DIR)/src
LDLIBS := $(LIBS:%=-l%)
ifeq ($(BUILD_TYPE), RELEASE)
	RAYLIB := $(RAYLIB_DIR)/src/libraylib.a
	LDLIBS += -lraylib
	ifeq ($(OS), Windows_NT)
		LDLIBS += -lgdi32 -lwinmm
	endif
else ifeq ($(OS), Windows_NT)
	RAYLIB := $(RAYLIB_DIR)/src/raylib$(SO_EXT)
	LDLIBS += -lraylibdll
else
	RAYLIB := $(RAYLIB_DIR)/src/libraylib$(SO_EXT)
	LDLIBS += -lraylib
endif

# prevent binary from linking to shared library
ifeq ($(BUILD_TYPE), RELEASE)
	ifeq ($(OS), Windows_NT)
		RM_RAYLIB_SO := $(RM) $(RAYLIB_DIR)/src/raylib$(SO_EXT)
		RM_RAYLIB_SO_STUB := $(RM) $(RAYLIB_DIR)/src/libraylibdll.a
	else
		RM_RAYLIB_SO := $(RM) $(RAYLIB_DIR)/src/libraylib$(SO_EXT)
	endif
else
	BUILD_SO := $(CXX) -fPIC -shared $(LDFLAGS) $(OBJ) $(LDLIBS) -o $(SO)
endif

.PHONY: all clean

all: $(OBJ)
	$(RM_RAYLIB_SO)
	$(RM_RAYLIB_SO_STUB)
	$(BUILD_SO)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $(BIN)

clean:
	$(RM) -r $(BUILD_DIR)

$(BUILD_DIR)/%.o: src/%.cpp | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(dir $@)
	mkdir -p $@

-include $(OBJ:.o=.d)
