CURRENT_DIR=$(shell pwd)
FP_GROWTH_INCLUDE=$(CURRENT_DIR)/include
BUILD_DIR=$(CURRENT_DIR)/build
BINARY_DIR=$(CURRENT_DIR)/bin
SRC_DIR=$(CURRENT_DIR)/src
BINARY_NAME:=fp_growth
CXX:=g++
CXXFLAG=-I$(FP_GROWTH_INCLUDE)

$(BINARY_DIR)/$(BINARY_NAME): $(BUILD_DIR)/fp_growth.o
	$(CXX) -o $(BINARY_DIR)/$(BINARY_NAME) $(BUILD_DIR)/fp_growth.o

$(BUILD_DIR)/fp_growth.o: $(SRC_DIR)/fp_growth.cc
	$(CXX) -c $(CXXFLAG) $(SRC_DIR)/fp_growth.cc -o $(BUILD_DIR)/fp_growth.o


clean: 
	rm -rf build/* bin/*

.PHONY: clean


test:
	$(BINARY_DIR)/$(BINARY_NAME)
.PHONY: test