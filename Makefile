PROJECT_NAME := 3dRenderer

BUILD_DIR ?= build/debug

PROJECT_SOURCE_FILES := $(wildcard src/*.cpp)
PROJECT_OBJECT_FILES = $(patsubst src/%.cpp, $(BUILD_DIR)/%.o, $(PROJECT_SOURCE_FILES))
EXECUTABLE = $(BUILD_DIR)/$(PROJECT_NAME)

SDW_DIR := ./libs/sdw/
GLM_DIR := ./libs/glm-0.9.7.2/
SDW_SOURCE_FILES := $(wildcard $(SDW_DIR)*.cpp)
SDW_OBJECT_FILES = $(patsubst $(SDW_DIR)%.cpp, $(BUILD_DIR)/%.o, $(SDW_SOURCE_FILES))

# Build settings
COMPILER := clang++
COMPILER_OPTIONS := -c -pipe -Wall -std=c++11 

# Options (defined as variables for clarity)
DEBUG_OPTIONS := -ggdb -g3
FUSSY_OPTIONS := -Werror -pedantic
SANITIZER_OPTIONS := -O1 -fsanitize=undefined -fsanitize=address -fno-omit-frame-pointer
SPEEDY_OPTIONS := -Ofast -funsafe-math-optimizations -march=native
LINKER_OPTIONS :=

# Flags
SDW_COMPILER_FLAGS := -I$(SDW_DIR)
GLM_COMPILER_FLAGS := -I$(GLM_DIR)
SDL_COMPILER_FLAGS := $(shell sdl2-config --cflags)
SDL_LINKER_FLAGS := $(shell sdl2-config --libs)

# Default target
default: debug

debug:
	@$(MAKE) binary BUILD_DIR=build/debug EXTRA_FLAGS="$(DEBUG_OPTIONS)"

diagnostic:
	@$(MAKE) binary BUILD_DIR=build/diagnostic EXTRA_FLAGS="$(FUSSY_OPTIONS) $(SANITIZER_OPTIONS)"

speedy:
	@$(MAKE) binary BUILD_DIR=build/speedy EXTRA_FLAGS="$(SPEEDY_OPTIONS)"

production:
	@$(MAKE) binary BUILD_DIR=build/production EXTRA_FLAGS=""


binary: directories $(EXECUTABLE)
	./$(EXECUTABLE)

directories:
	@mkdir -p $(BUILD_DIR)

# Link the executable
$(EXECUTABLE): $(PROJECT_OBJECT_FILES) $(SDW_OBJECT_FILES)
	$(COMPILER) $(LINKER_OPTIONS) $(EXTRA_FLAGS) -o $(EXECUTABLE) $^ $(SDL_LINKER_FLAGS)

# Rule for building project source files
$(BUILD_DIR)/%.o: src/%.cpp
	$(COMPILER) $(COMPILER_OPTIONS) $(EXTRA_FLAGS) -c -o $@ $< $(SDW_COMPILER_FLAGS) $(SDL_COMPILER_FLAGS) $(GLM_COMPILER_FLAGS)

# Rule for building SDW source files
$(BUILD_DIR)/%.o: $(SDW_DIR)%.cpp
	$(COMPILER) $(COMPILER_OPTIONS) -c -o $@ $< $(SDL_COMPILER_FLAGS) $(GLM_COMPILER_FLAGS)

# Clean rule
clean:
	rm -rf build/*
