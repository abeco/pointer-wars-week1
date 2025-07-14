# ===========================
# Compiler and Flags
# ===========================
CC := gcc

FLAGS := -std=c99 # -fsanitize=address
WARNINGS := -Wall -Wextra -Werror
OPTIMIZATIONS := -O3 -g
CFLAGS := $(FLAGS) $(WARNINGS) $(OPTIMIZATIONS)

LDFLAGS :=  #-fsanitize=address

SO_FLAGS := -shared -fPIC -g

# ===========================
# Directories
# ===========================
BUILD_DIR := build

# ===========================
# Source and Object Files
# ===========================
LINKED_LIST_SRC := linked_list.c
LINKED_LIST_OBJ := $(BUILD_DIR)/linked_list.o

TEST_SRC := linked_list_test_program.c
TEST_OBJ := $(BUILD_DIR)/linked_list_test_program.o

# ===========================
# Targets
# ===========================

# Build shared library
liblinked_list.so: $(LINKED_LIST_OBJ)
	$(CC) $(CFLAGS) $(SO_FLAGS) $^ -o $@

# Build test executable
linked_list_test_program: $(TEST_OBJ) $(LINKED_LIST_OBJ)
ifeq ($(PLATFORM),UNIX)
	$(CC) $(LDFLAGS) -o $@ $(TEST_OBJ) -L. -llinked_list
else
	$(CC) $(LDFLAGS) -o $@ $(TEST_OBJ) $(LINKED_LIST_OBJ)
endif


# Run functional tests
run_functional_tests: linked_list_test_program
	LD_LIBRARY_PATH=`pwd`:$$LD_LIBRARY_PATH ./linked_list_test_program

# Run functional tests under GDB
run_functional_tests_gdb: linked_list_test_program
	LD_LIBRARY_PATH=`pwd`:$$LD_LIBRARY_PATH gdb ./linked_list_test_program

# Run functional tests under Valgrind
run_valgrind_tests: linked_list_test_program
	LD_LIBRARY_PATH=`pwd`:$$LD_LIBRARY_PATH valgrind ./linked_list_test_program

# Compilation rule with automatic build directory creation
$(BUILD_DIR)/%.o: %.c
	mkdir -p $(BUILD_DIR)
	$(CC) -c $(CFLAGS) $< -o $@

# Clean
clean:
	rm -rf $(BUILD_DIR) liblinked_list.so linked_list_test_program linked_list_test_program.exe

.PHONY: clean run_functional_tests run_functional_tests_gdb run_valgrind_tests
