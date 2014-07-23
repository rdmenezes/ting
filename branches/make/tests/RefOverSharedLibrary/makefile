define TODO_write_makefile
$(info entered tests/RefOverSharedLibrary/makefile)

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


$(info left tests/RefOverSharedLibrary/makefile)





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
    libs := ../../src/libting.dll -mwindows
else
    libs := ../../src/libting.so -pthread
endif

#external compiler flags (to be passed to make from command line)
ecf :=

#compiler flags
cf := $(ecf)
cf += -Wall
cf += -DDEBUG
cf += -fstrict-aliasing #strict aliasing!!!


default: $(binary_name)

test: $(binary_name)
	@echo running $^...
ifeq ($(platform),windows)
	@cp ../../src/libting.dll .
	@$^
else
    ifeq ($(platform),macosx)
	@DYLD_LIBRARY_PATH=../../src:. ./$^
    else
	@LD_LIBRARY_PATH=../../src:. ./$^
    endif
endif




lib$(lib_name).$(lib_ext): testso.cpp
ifeq ($(platform),windows)
	@g++ $< -shared -o lib$(lib_name).$(lib_ext) $(cf)
else
#use -fPIC (position-independent code) for shared libraries.
	@g++ $< -shared -fPIC -o lib$(lib_name).$(lib_ext) $(cf)
endif




$(binary_name): Ref_test.cpp lib$(lib_name).$(lib_ext)
	@echo Building $@...
	@g++ -o $@ $(cf) $< $(libs) -L. -l$(lib_name)



clean:
	@rm -rf $(binary_name)
	@rm -rf lib$(lib_name).$(lib_ext)

include ../../targets.mk
endef