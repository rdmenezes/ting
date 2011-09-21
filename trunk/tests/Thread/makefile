platform := linux

binary_name := Thread_test

ifeq ($(platform),win32)
binary_name := $(binary_name).exe
endif


ifeq ($(platform),win32)
    libs := 
    remove := del /Q /F 
endif
ifeq ($(platform),darwin)
    libs := 
    remove := rm -rf
endif
ifeq ($(platform),linux)
    libs := ../../src/libting.so
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
	@LD_LIBRARY_PATH=../../src ./$^

$(binary_name): Thread_test.cpp
	@echo Building $@...
	@g++ -o $@ $(cf) $< $(libs)



clean:
	@$(remove) $(binary_name)

include ../../targets.mk
