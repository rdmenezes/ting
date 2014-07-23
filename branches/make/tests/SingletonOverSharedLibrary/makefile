$(info entered tests/SingletonOverSharedLibrary/makefile)

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








define TODO_remove_after_testing_under_windows
include ../../platform.mk

binary_name := tests
lib_name := testso

ifeq ($(platform),windows)
    binary_name := $(binary_name).exe
    lib_ext := dll
else
    lib_ext := so
endif


ifeq ($(platform),windows)
    libs := -mwindows
else
    libs :=
endif

#external compiler flags (to be passed to make from command line)
ecf :=

#compiler flags
cf := $(ecf)
cf += -Wall
cf += -DDEBUG
cf += -fstrict-aliasing #strict aliasing!!!

ifeq ($(platform),windows)
else
    cf += -fPIC
endif


default: $(binary_name)


test: $(binary_name)
	@echo running $^...
ifeq ($(platform),windows)
	@./$^
else
	@LD_LIBRARY_PATH=. ./$^
endif


lib$(lib_name).$(lib_ext): testso.cpp
#use -fPIC (position-independent code) for shared libraries.
	@g++ $< -shared -o lib$(lib_name).$(lib_ext) $(cf)




$(binary_name): Singleton_test.cpp lib$(lib_name).$(lib_ext)
	@echo Building $@...
	@g++ -o $@ $(cf) $< $(libs) -L. -l$(lib_name)



clean:
	@rm -rf $(binary_name)
	@rm -rf lib$(lib_name).$(lib_ext)

include ../../targets.mk

	
endef