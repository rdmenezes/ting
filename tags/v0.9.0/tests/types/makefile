platform := linux

binary_name := types_test

ifeq ($(platform),win32)
binary_name := $(binary_name).exe
endif


ifeq ($(platform),win32)
    libs := -lws2_32
    remove := del /Q /F
endif
ifeq ($(platform),darwin)
    libs := -lpthread
    remove := rm -rf
endif
ifeq ($(platform),linux)
    libs := -lpthread -lrt
    remove := rm -rf
endif

#external compiler flags (to be passed to make from command line)
ecf :=

#compiler flags
cf := $(ecf)
cf += -Wall
cf += -DDEBUG
cf += -fstrict-aliasing #strict aliasing!!!

default: $(binary_name)

test: $(binary_name)
	@echo running $^...
	@./$^

$(binary_name): types_test.cpp
	@echo Building $@...
	@g++ -o $@ $(cf) $< $(libs)



clean:
	@$(remove) $(binary_name)

include ../../targets.mk

