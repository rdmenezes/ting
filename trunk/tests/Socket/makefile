platform := linux

binary_name := tests

default: $(binary_name)



ifeq ($(platform),win32)
binary_name := $(binary_name).exe
endif


ifeq ($(platform),win32)
    libs := -lws2_32
    remove := del /Q /F
endif
ifeq ($(platform),darwin)
    libs := 
    remove := rm -rf
endif
ifeq ($(platform),linux)
    libs :=
    remove := rm -rf
endif


#external compiler flags (to be passed to make from command line)
ecf := -DDEBUG

#compiler flags
cf := $(ecf)
cf += -Wall
cf += -g


test: $(binary_name)
	@echo running $^...
ifeq ($(platform),win32)
	@./$^
else
	@LD_LIBRARY_PATH=../../src ./$^
endif


%.o:%.cpp
	@echo Compiling $<...
	@g++ -c -o $@ $(cf) $<

include ../libting.mk

$(binary_name): socket.o dns.o main.o $(libting)
	@echo Linking $@...
	@g++ $^ -o $@ $(cf) $(libs)



clean:
	@$(remove) $(binary_name)
	@$(remove) *.o

include ../../targets.mk
