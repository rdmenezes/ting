platform := linux

operating_system := $(shell uname -o)
ifeq ($(operating_system), Msys)
    platform := win32
endif

binary_name := tests

ifeq ($(platform),win32)
binary_name := $(binary_name).exe
endif

default: $(binary_name)

ifeq ($(platform),win32)
    libs := ../../src/libting.a
endif
ifeq ($(platform),darwin)
    libs := 
endif
ifeq ($(platform),linux)
    libs := ../../src/libting.so
endif

#external compiler flags (to be passed to make from command line)
ecf :=

#compiler flags
cf := $(ecf)
cf += -Wall
cf += -DDEBUG
cf += -fstrict-aliasing #strict aliasing!!!



test: $(binary_name)
	@echo running $^...
ifeq ($(platform),win32)
	@./$^
else
	@LD_LIBRARY_PATH=../../src ./$^
endif

include ../libting.mk

$(binary_name): main.cpp tests.cpp $(libting)
	@echo Building $@...
	@g++ -o $@ $(cf) $^ $(libs)



clean:
	@rm -rf $(binary_name)

include ../../targets.mk

