$(info entered tests/Flags/makefile)

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



$(eval $(prorab-build-app))

include $(prorab_this_dir)../test_target.mk


$(info left tests/Flags/makefile)
