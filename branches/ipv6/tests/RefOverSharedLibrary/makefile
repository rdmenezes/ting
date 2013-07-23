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
    libs := ../../src/libting.so
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
	@LD_LIBRARY_PATH=../../src:. ./$^
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
