BUILD_DIR := make-build
LIB := libseb.a
SRC := $(wildcard src/*.cpp)
OBJ := $(SRC:src/%.cpp=$(BUILD_DIR)/%.o)

CPPFLAGS := -MMD -MP -isystem$(RAYLIB_DIR)/src -iquote$(RAYLIB_CPP_DIR)/include
CXXFLAGS := -O3 -Wall -Wextra -Wpedantic -Werror -std=c++23

.PHONY: all clean

all: $(LIB)

clean:
	$(RM) -r make-build
	$(RM) $(LIB)

$(LIB): $(OBJ) | make-build
	ar rcs $(LIB) $(OBJ)

$(BUILD_DIR)/%.o: src/%.cpp | make-build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir $@

-include $(OBJ:.o=.d)
