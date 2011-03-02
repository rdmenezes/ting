platform := linux

binary_name := Ref_test

ifeq ($(platform),win32)
binary_name := $(binary_name).exe
endif


ifeq ($(platform),win32)
    libs := 
    remove := del /Q /F
else
    libs := 
    remove := rm -rf
endif



#compiler flags
cf := -Wall -I../../src
cf += -DDEBUG
cf += -fstrict-aliasing #strict aliasing!!!


default: $(binary_name)
	@echo running $^...
	@LD_LIBRARY_PATH=. ./$^




libtestso.so: testso.cpp
	@g++ $< -shared -o libtestso.so $(cf)




$(binary_name): Ref_test.cpp libtestso.so
	@echo Building $@...
	@g++ -o $@ $(cf) $< $(libs) -L. -ltestso



clean:
	@$(remove) $(binary_name)
	@$(remove) libtestso.so
	