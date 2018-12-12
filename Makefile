CXX      := g++
CXXFLAGS := -std=c++11 -pedantic-errors -Wall -Wextra -Werror -mrtm
LDFLAGS  :=
BUILD    := ./build
TEST_DIR  := $(BUILD)/tests
OBJ_DIR  := $(BUILD)/objects
TARGET   := program
INCLUDE  := src/
SRC      :=                      \
	$(wildcard src/*.cpp)
TEST     :=                      \
	$(wildcard test/*.cpp)

OBJECTS := $(SRC:%.cpp=$(OBJ_DIR)/%.o)
# TESTS   := $(TEST:%.cpp=$(TEST_DIR)/%.out)

all: build $(TEST_DIR)/main

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(TEST_DIR)/main: $(OBJECTS) $(TEST)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -I $(INCLUDE) $(LDFLAGS) -o $(TEST_DIR)/main test/main.cpp src/concurrent_hashmap.cpp #$(OBJECTS)#

.PHONY: build clean debug release

build:
	@mkdir -p $(TEST_DIR)
	@mkdir -p $(OBJ_DIR)

debug: CXXFLAGS += -DDEBUG -g
debug: all

release: CXXFLAGS += -O2
release: all

clean:
	-@rm -rvf $(OBJ_DIR)/*
	-@rm -rvf $(TEST_DIR)/*
