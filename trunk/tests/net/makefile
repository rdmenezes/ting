platform := linux

operating_system := $(shell uname)
ifeq ($(operating_system), Msys)
    platform := win32
endif

binary_name := tests

ifeq ($(platform),win32)
binary_name := $(binary_name).exe
endif

default: $(binary_name)

ifeq ($(platform),win32)
    libs := -lws2_32
endif
ifeq ($(platform),darwin)
    libs := 
endif
ifeq ($(platform),linux)
    libs :=
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
	@rm -rf $(binary_name)
	@rm -rf *.o

include ../../targets.mk
