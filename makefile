# Makefile — EduSec Toolkit (PIA PAC Ene-Jun 2026)
# Compilador y flags: C++17, warnings altos, sin dependencias externas.

CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -O2 -Isrc
LDFLAGS  :=

SRC_DIR   := src
BUILD_DIR := build
TARGET    := $(BUILD_DIR)/edusec

# Recolecta todos los .cpp recursivamente y mapea a .o dentro de build/
SOURCES := $(shell find $(SRC_DIR) -name '*.cpp')
OBJECTS := $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJECTS)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)
	@echo "==> Binario listo: $@"

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET) --help

clean:
	rm -rf $(BUILD_DIR)
	@echo "==> build/ eliminado"
