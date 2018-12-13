CXX      := g++
CXXFLAGS := -std=c++11 -pedantic-errors -Wall -Wextra -Werror -mrtm -g
LDFLAGS  := -lbenchmark -lpthread
BUILD    := ./build
TEST_DIR  := $(BUILD)/tests
BENCH_DIR  := $(BUILD)/bench
OBJ_DIR  := $(BUILD)/objects
TARGET   := program
INCLUDE  := src/
SRC      :=                      \
	$(wildcard src/*.cpp)
TEST     :=                      \
	$(wildcard test/*.cpp)
BENCH     :=                     \
	$(wildcard bench/*.cpp)

OBJECTS := $(SRC:%.cpp=$(OBJ_DIR)/%.o)
# TESTS   := $(TEST:%.cpp=$(TEST_DIR)/%.out)

all: build $(TEST_DIR)/main $(BENCH_DIR)/main

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(TEST_DIR)/main: $(OBJECTS) $(TEST)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -I $(INCLUDE) $(LDFLAGS) -o $(TEST_DIR)/main test/main.cpp src/concurrent_hashmap.cpp src/concurrent_tm_hashmap.cpp src/sequential_hashmap.cpp #$(OBJECTS)#

$(BENCH_DIR)/main: $(OBJECTS) $(BENCH)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -L benchmark/build/src/ -I $(INCLUDE) $(LDFLAGS) -o $(BENCH_DIR)/main bench/main.cpp src/concurrent_hashmap.cpp #$(OBJECTS)#

.PHONY: build clean debug release

build:
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(TEST_DIR)
	@mkdir -p $(BENCH_DIR)

debug: CXXFLAGS += -DDEBUG -g
debug: all

release: CXXFLAGS += -O2
release: all

clean:
	-@rm -rvf $(OBJ_DIR)/*
	-@rm -rvf $(TEST_DIR)/*
	-@rm -rvf $(BENCH_DIR)/*
