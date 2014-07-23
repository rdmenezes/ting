

$(info entered tests/Signal/makefile)

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


$(eval $(prorab-build-app))


$(info left tests/Signal/makefile)








define TODO_write_fwefwe
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