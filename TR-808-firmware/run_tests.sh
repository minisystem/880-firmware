#!/bin/bash

# Compiler and flags
CC=gcc
CFLAGS="-Wall -Werror -std=c99"  # Add more flags as needed for your project

# Source files
SOURCE_FILES="select_instrument_with_soloing.c unity.c test_select_instruments.c"

# Output executable
EXECUTABLE=test_select_instruments

# Compilation step
$CC $CFLAGS $SOURCE_FILES -o $EXECUTABLE

# Check for compilation errors
if [ $? -ne 0 ]; then
    echo "Compilation failed!"
    exit 1
fi

# Run the tests
./$EXECUTABLE
rm ./$EXECUTABLE

# Check for test failures
if [ $? -ne 0 ]; then
    echo "Tests failed!"
    exit 1
fi

echo "All tests passed!"
