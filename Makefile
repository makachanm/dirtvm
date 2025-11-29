CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g

# Directories
ENGINE_DIR = engine
ASSEMBLER_DIR = assembler
CLI_DIR = cli

# Engine Test Sources
ENGINE_TEST_SRC = $(ENGINE_DIR)/test.cpp
ENGINE_VM_SRC = $(ENGINE_DIR)/vm.cpp
ENGINE_OBJECT_SRC = $(ENGINE_DIR)/object.cpp
ENGINE_SYSCALL_SRC = $(ENGINE_DIR)/syscall.cpp # Assuming vm.cpp might use this

# Assembler Test Sources
ASSEMBLER_TEST_SRC = $(ASSEMBLER_DIR)/parser_test.cpp
ASSEMBLER_PARSER_SRC = $(ASSEMBLER_DIR)/parser.cpp

# CLI Sources
CLI_MAIN_SRC = $(CLI_DIR)/main.cpp

# Executables
ENGINE_TEST_BIN = $(ENGINE_DIR)/engine_test
ASSEMBLER_TEST_BIN = $(ASSEMBLER_DIR)/assembler_test
CLI_BIN = $(CLI_DIR)/dirtvm_cli

.PHONY: all clean test

all: $(ENGINE_TEST_BIN) $(ASSEMBLER_TEST_BIN) $(CLI_BIN)

$(ENGINE_TEST_BIN): $(ENGINE_TEST_SRC) $(ENGINE_VM_SRC) $(ENGINE_OBJECT_SRC) $(ENGINE_SYSCALL_SRC)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(ASSEMBLER_TEST_BIN): $(ASSEMBLER_TEST_SRC) $(ASSEMBLER_PARSER_SRC)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(CLI_BIN): $(CLI_MAIN_SRC) $(ASSEMBLER_PARSER_SRC) $(ENGINE_VM_SRC) $(ENGINE_OBJECT_SRC) $(ENGINE_SYSCALL_SRC)
	$(CXX) $(CXXFLAGS) $^ -o $@

test: $(ENGINE_TEST_BIN) $(ASSEMBLER_TEST_BIN)
	@echo "Running Engine Tests..."
	./$(ENGINE_TEST_BIN)
	@echo "\nRunning Assembler Tests..."
	./$(ASSEMBLER_TEST_BIN)

clean:
	rm -f $(ENGINE_TEST_BIN) $(ASSEMBLER_TEST_BIN) $(CLI_BIN)
	rm -f $(ASSEMBLER_DIR)/*.o $(ENGINE_DIR)/*.o $(CLI_DIR)/*.o # Remove any potential object files
