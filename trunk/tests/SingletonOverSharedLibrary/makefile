$(info entered tests/SingletonOverSharedLibrary/makefile)

#this should be the first include
ifeq ($(prorab_included),true)
    include $(prorab_dir)prorab.mk
else
    include ../../prorab.mk
endif



this_name := testso
this_so_name := 0

this_cflags += -std=c++11
this_cflags += -Wall
this_cflags += #-DDEBUG
this_cflags += -fstrict-aliasing #strict aliasing!!!


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
this_cflags += -std=c++11
this_cflags += -Wall
this_cflags += -DDEBUG
this_cflags += -fstrict-aliasing #strict aliasing!!!

this_srcs := Singleton_test.cpp

this_ldlibs := $(lib_target_name)


$(eval $(prorab-build-app))

$(prorab_this_name): $(lib_target_name)


#no test under windows, it will fails there
ifneq ($(prorab_os),windows)
define this_rules
test:: $(prorab_this_name)
	@echo running $$^...
	@(cd $(prorab_this_dir); $(if $(filter windows,$(prorab_os)), $$^, LD_LIBRARY_PATH=. $$^))
endef
$(eval $(this_rules))
endif


$(info left tests/SingletonOverSharedLibrary/makefile)
