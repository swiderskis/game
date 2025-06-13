export LIBS := seb-engine seblib
export SEBLIB_DIR := lib/seblib
export RAYLIB_DIR := ext-lib/raylib
export RAYLIB_CPP_DIR := ext-lib/raylib-cpp
export BIN_EXT :=
export SO_EXT := .so
ifeq ($(OS), Windows_NT)
	BIN_EXT := .exe
	SO_EXT := .dll
endif
export BIN := game$(BIN_EXT)

ifeq ($(OS), Windows_NT)
	CP_RAYLIB_SO := cp $(RAYLIB_DIR)/src/raylib$(SO_EXT) build/debug
else
	export LD_LIBRARY_PATH := $(RAYLIB_DIR)/src:$(LD_LIBRARY_PATH)
endif

.PHONY: all run debug release clean clean_libs clean_raylib

all: debug

run: debug
	$(CP_RAYLIB_SO)
	./build/debug/$(BIN)

debug:
	$(MAKE) PLATFORM=PLATFORM_DESKTOP RAYLIB_LIBTYPE=SHARED -C$(RAYLIB_DIR)/src
	$(MAKE) BUILD_TYPE=DEBUG BUILD_DIR=build/debug -Clib
	$(MAKE) BUILD_TYPE=DEBUG BUILD_DIR=build/debug -fMakemain.mk

release:
	$(MAKE) PLATFORM=PLATFORM_DESKTOP -C$(RAYLIB_DIR)/src
	$(MAKE) BUILD_TYPE=RELEASE BUILD_DIR=build/release -Clib
	$(MAKE) BUILD_TYPE=RELEASE BUILD_DIR=build/release -fMakemain.mk

clean:
	$(MAKE) clean BUILD_DIR=build -fMakemain.mk

clean_libs: clean
	$(MAKE) clean BUILD_DIR=build -Clib

clean_raylib: clean_libs
	$(MAKE) clean -C$(RAYLIB_DIR)/src
