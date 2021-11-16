CURRENT_DIR        :=$(shell pwd)
FP_GROWTH_INCLUDE  =$(CURRENT_DIR)/include
BUILD_DIR          =$(CURRENT_DIR)/build
BINARY_DIR         :=$(CURRENT_DIR)/bin
SRC_DIR            :=$(CURRENT_DIR)/src
BINARY             :=$(BINARY_DIR)/fp_growth
CXX                :=g++
CXXFLAG            :=-I$(FP_GROWTH_INCLUDE) -std=c++11 
LD_FLAGS           :=-lgflags -lpthread -lglog
SRCS               := $(shell find $(SRC_DIR) -type f -name "*.cc" | sort)
SRC_NAMES          :=$(notdir $(SRCS))
OBJS               :=$(addprefix $(BUILD_DIR)/,$(SRC_NAMES))
OBJS               :=$(OBJS:.cc=.o)


$(BINARY): $(OBJS)
	@echo Linking $@
	@mkdir -p $(@D)
	@$(CXX) $^ -o $@  $(CXXFLAG) $(LD_FLAGS)

$(BUILD_DIR)/%.o: src/%.cc
	@echo Compiling $<
	@mkdir -p $(@D)
	@$(CXX) $(CXXFLAG) -c -o $@ $<


clean: 
	rm -rf $(BUILD_DIR)/* $(BINARY_DIR)/*

.PHONY: clean


test:
	$(BINARY)
.PHONY: test