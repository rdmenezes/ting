define TODO_wefwfqeferw

$(info entered tests/net/makefile)

#this should be the first include
ifeq ($(prorab_included),true)
    include $(prorab_dir)prorab.mk
else
    include ../../prorab.mk
endif



this_name := tests


#compiler flags
this_cflags :=
this_cflags += -Wall
this_cflags += -DDEBUG
this_cflags += -fstrict-aliasing #strict aliasing!!!

this_srcs := main.cpp tests.cpp



#$(info prorab_this_dir = $(prorab_this_dir))
include $(prorab_this_dir)../test_target.mk


$(eval $(prorab-app-rules))


$(info left tests/net/makefile)








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
    libs := -pthread -lrt
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


endef