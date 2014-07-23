$(info entered tests/atomic/makefile)

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



this_ldlibs := -lting -lpthread

this_ldflags := -L$(prorab_this_dir)../../src/

#add dependency on libting.so
$(prorab_this_dir)tests: $(prorab_this_dir)../../src/libting.so



#$(info prorab_this_dir = $(prorab_this_dir))
include $(prorab_this_dir)../test_target.mk


#$(info $(prorab-app-rules))
$(eval $(prorab-app-rules))


#include makefile for building ting
$(eval $(call prorab-include,$(prorab_this_dir)../../src/makefile))


$(info left tests/atomic/makefile)
