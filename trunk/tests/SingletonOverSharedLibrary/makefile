platform := linux

binary_name := Singleton_test
lib_name := testso

ifeq ($(platform),win32)

binary_name := $(binary_name).exe
lib_ext := dll

else

lib_ext := so

endif


ifeq ($(platform),win32)
    libs := -mwindows
    remove := del /Q /F
else
    libs := 
    remove := rm -rf
endif

#external compiler flags (to be passed to make from command line)
ecf :=

#compiler flags
cf := $(ecf)
cf += -Wall -I../../src
cf += -DDEBUG
cf += -fstrict-aliasing #strict aliasing!!!


default: $(binary_name)

test: $(binary_name)
	@echo running $^...
ifeq ($(platform),win32)
	@./$^
else
	@LD_LIBRARY_PATH=. ./$^
endif




lib$(lib_name).$(lib_ext): testso.cpp
	@g++ $< -shared -o lib$(lib_name).$(lib_ext) $(cf)




$(binary_name): Singleton_test.cpp lib$(lib_name).$(lib_ext)
	@echo Building $@...
	@g++ -o $@ $(cf) $< $(libs) -L. -l$(lib_name)



clean:
	@$(remove) $(binary_name)
	@$(remove) lib$(lib_name).$(lib_ext)
	