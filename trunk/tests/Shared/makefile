$(info entered tests/Ref/makefile)

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
this_cflags += -std=c++11

this_srcs := main.cpp tests.cpp

this_ldlibs := -lting

ifeq ($(prorab_os),windows)
else
    this_ldlibs += -lpthread
endif

this_ldflags := -L$(prorab_this_dir)../../src/

#add dependency on libting.so
$(abspath $(prorab_this_dir)tests): $(abspath $(prorab_this_dir)../../src/libting$(prorab_lib_extension))


$(eval $(prorab-build-app))

include $(prorab_this_dir)../test_target.mk


#include makefile for building ting
$(eval $(call prorab-include,$(prorab_this_dir)../../src/makefile))

$(info left tests/Ref/makefile)
