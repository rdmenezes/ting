platform := linux

binary_name := Socket_test

ifeq ($(platform),win32)
binary_name := $(binary_name).exe
endif


ifeq ($(platform),win32)
    remove := del /Q /F 
else
    remove := rm -rf
endif



#compiler flags
cf := -Wall -I../../src
cf += -DDEBUG


default: $(binary_name)
	@echo running Socket test...
	@./$^

$(binary_name): Socket_test.cpp
	@echo Building $@...
	@g++ -o $@ $(cf) -lpthread -lrt $<



clean:
	@$(remove) $(binary_name)


