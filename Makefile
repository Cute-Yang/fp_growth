CURRENT_DIR=$(shell pwd)
FP_GROWTH_INCLUDE=$(CURRENT_DIR)/include
BUILD_DIR=$(CURRENT_DIR)/build
BINARY_DIR=$(CURRENT_DIR)/bin
SRC_DIR=$(CURRENT_DIR)/src
BINARY_NAME:=fp_growth_v2
GLOG_LIB:=/usr/local/lib
GLOG_NAME:=glog
CXX:=g++
CXXFLAG=-I$(FP_GROWTH_INCLUDE) -L$(GLOG_LIB) -l$(GLOG_NAME) -lgflags -lpthread -std=c++11

$(BINARY_DIR)/$(BINARY_NAME): $(BUILD_DIR)/fp_growth_v2.o
	$(CXX) $(BUILD_DIR)/fp_growth_v2.o -o $(BINARY_DIR)/$(BINARY_NAME) $(CXXFLAG)

$(BUILD_DIR)/fp_growth_v2.o: $(SRC_DIR)/fp_growth_v2.cc
	$(CXX) -c $(CXXFLAG) $(SRC_DIR)/fp_growth_v2.cc -o $(BUILD_DIR)/fp_growth_v2.o


clean: 
	rm -rf build/* bin/*

.PHONY: clean


test:
	$(BINARY_DIR)/$(BINARY_NAME)
.PHONY: test