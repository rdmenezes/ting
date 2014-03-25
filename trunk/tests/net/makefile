include ../../platform.mk

binary_name := tests

ifeq ($(platform),windows)
    binary_name := $(binary_name).exe
endif

default: $(binary_name)

ifeq ($(platform),windows)
    libs := -lws2_32
endif
ifeq ($(platform),macosx)
    libs := 
endif
ifeq ($(platform),linux)
    libs := -pthread
endif


#external compiler flags (to be passed to make from command line)
ecf := -DDEBUG

#compiler flags
cf := $(ecf)
cf += -Wall
cf += -g


include ../test_target.mk


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
