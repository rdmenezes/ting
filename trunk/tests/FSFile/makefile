$(info entered tests/FSFile/makefile)

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


this_ldlibs := -lting

this_ldflags := -L$(prorab_this_dir)../../src/



$(eval $(prorab-build-app))

include $(prorab_this_dir)../test_target.mk


#add dependency on libting
this_target_name := $(prorab_this_name): $(abspath $(prorab_this_dir)../../src/libting$(prorab_lib_extension))


#include makefile for building ting
$(eval $(call prorab-include,$(prorab_this_dir)../../src/makefile))


$(info left tests/FSFile/makefile)
