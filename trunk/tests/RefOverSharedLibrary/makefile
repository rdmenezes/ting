$(info entered tests/RefOverSharedLibrary/makefile)

#this should be the first include
ifeq ($(prorab_included),true)
    include $(prorab_dir)prorab.mk
else
    include ../../prorab.mk
endif



this_name := testso
this_so_name := 0

this_cflags :=
this_cflags += -Wall
this_cflags += #-DDEBUG
this_cflags += -fstrict-aliasing #strict aliasing!!!
this_cflags += -std=c++11

ifeq ($(prorab_os),windows)
else
    this_cflags += -fPIC
endif

this_srcs := testso.cpp

#$(info $(prorab-build-lib))
$(eval $(prorab-build-lib))

lib_target_name := $(prorab_this_name)



$(prorab-clear-this-vars)



this_name := tests


#compiler flags
this_cflags :=
this_cflags += -Wall
this_cflags += -DDEBUG
this_cflags += -fstrict-aliasing #strict aliasing!!!
this_cflags += -std=c++11

this_srcs := Ref_test.cpp

this_ldflags := -L$(prorab_this_dir)../../src/

this_ldlibs := $(lib_target_name) -lting

ifeq ($(prorab_os),windows)
else
    this_cflags += -fPIC
    this_ldlibs := -lpthread
endif


$(eval $(prorab-build-app))

$(prorab_this_name): $(lib_target_name)


define this_rules
test:: $(prorab_this_name)
	@echo running $$^...
	@(cd $(prorab_this_dir); $(if $(filter windows,$(prorab_os)), $$^, LD_LIBRARY_PATH=.:../../src $$^))
endef
$(eval $(this_rules))



#add dependency on libting.so
$(prorab_this_name): $(abspath $(prorab_this_dir)../../src/libting$(prorab_lib_extension))


#include makefile for building ting
$(eval $(call prorab-include,$(prorab_this_dir)../../src/makefile))



$(info left tests/RefOverSharedLibrary/makefile)
