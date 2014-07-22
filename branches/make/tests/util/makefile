$(info entered tests/util/makefile)

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


$(info left tests/util/makefile)
