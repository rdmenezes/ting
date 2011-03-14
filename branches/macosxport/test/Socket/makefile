platform := linux

binary_name := Socket_test

ifeq ($(platform),win32)
binary_name := $(binary_name).exe
endif


ifeq ($(platform),win32)
    libs := -lws2_32
    remove := del /Q /F 
endif
ifeq ($(platform),darwin)
    libs := -lpthread
    remove := rm -rf
endif
ifeq ($(platform),linux)
    libs := -lpthread -lrt
    remove := rm -rf
endif



#compiler flags
cf := -Wall -I../../src
cf += -DDEBUG


default: $(binary_name)
	@echo running $^...
	@./$^

$(binary_name): Socket_test.cpp
	@echo Building $@...
	@g++ -o $@ $(cf) $< $(libs)



clean:
	@$(remove) $(binary_name)


