LIB := seblib
LIB := $(addsuffix .a, $(addprefix $(BUILD_DIR)/lib, $(LIB)))
SRC := $(wildcard src/*.cpp)
OBJ := $(SRC:src/%.cpp=$(BUILD_DIR)/%.o)

CPPFLAGS := -MMD -MP
CPPFLAGS += -isystem../../$(RAYLIB_DIR)/src
CPPFLAGS += -iquote../../$(RAYLIB_CPP_DIR)/include
CXXFLAGS := -std=c++23
ifeq ($(BUILD_TYPE), RELEASE)
	CPPFLAGS += -DNDEBUG -DLOGLVL=1
	CXXFLAGS += -O3
else
	CPPFLAGS += -fPIC
	CXXFLAGS += -O2 -Wall -Wextra -Wpedantic -Werror
endif

.PHONY: all clean

all: $(OBJ)
	ar rcs $(LIB) $^

clean:
	$(RM) -r $(BUILD_DIR)

$(BUILD_DIR)/%.o: src/%.cpp | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(dir $@)
	mkdir -p $@

-include $(OBJ:.o=.d)
