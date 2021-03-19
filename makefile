cc = g++
c_flags = -g -Wall -Wextra -std=c++17
include_directory = vendor/json/single_include
# sources := $(wildcard *.cpp)
sources := main.cpp # I changed this cuz I get linker errors otherwise
EXECUTABLE := detective
BUILD_DIR := build/

.VPATH: .

.PHONY: build
$(BUILD_DIR)detective : $(sources)
	mkdir -p $(BUILD_DIR)
	$(cc) $(c_flags) -I $(include_directory) $(sources) -o $(BUILD_DIR)$(EXECUTABLE)

.PHONY: test
test :
	$(BUILD_DIR)$(EXECUTABLE) tests/input/*.json
	# Add diff check with expected output

.PHONY: clean
clean : 
	rm -rf $(BUILD_DIR)

.PHONY: format
format:
	indent -linux -l0 *.cpp