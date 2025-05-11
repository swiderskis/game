BUILD_DIR := build
LIB := libseblib.a
SRC := $(wildcard src/*.cpp)
OBJ := $(SRC:src/%.cpp=$(BUILD_DIR)/%.o)

CPPFLAGS := -fPIC -MMD -MP -isystem$(RAYLIB_DIR)/src -iquote$(RAYLIB_CPP_DIR)/include
CXXFLAGS := -O3 -Wall -Wextra -Wpedantic -Werror -std=c++23

.PHONY: all clean

all: $(LIB)

clean:
	$(RM) -r $(BUILD_DIR)
	$(RM) $(LIB)

$(LIB): $(OBJ) | $(BUILD_DIR)
	ar rcs $(LIB) $(OBJ)

$(BUILD_DIR)/%.o: src/%.cpp | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir $@

-include $(OBJ:.o=.d)
