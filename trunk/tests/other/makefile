platform := linux

ifeq ($(platform),linux)
ext :=
endif
ifeq ($(platform),win32)
ext := .exe
endif


ifeq ($(platform),win32)
    remove := del /Q /F 
else
    remove := rm -rf
endif



#libs
ifeq ($(platform),linux)
threadlib := -lpthread
rtlib := -lrt
endif
ifeq ($(platform),darwin)
threadlib := -lpthread
rtlib :=
endif
ifeq ($(platform),win32)
threadlib :=
rtlib :=
endif


compiler := g++

#external compiler flags (to be passed to make from command line)
ecf :=

#compiler flags
cf := $(ecf)
cf += -Wall
cf += -DDEBUG
cf += -g


default: utils_test$(ext) debug_test$(ext) Types_test$(ext) WaitSet_test$(ext)


test: run_debug_test run_utils_test run_Types_test run_WaitSet_test



run_utils_test: utils_test$(ext)
	@echo running $^...
	@./$^

utils_test$(ext): src/utils_test.cpp
	@echo Building $@...
	@$(compiler) -o $@ $(cf) $(threadlib) $(rtlib) $<




run_debug_test: debug_test$(ext)
	@echo running  $^...
	@./$^

debug_test$(ext): src/debug_test.cpp
	@echo Building $@...
	@$(compiler) -o $@ $(cf) $(threadlib) $(rtlib) $<



run_Types_test: Types_test$(ext)
	@echo running $^...
	@./$^

Types_test$(ext): src/Types_test.cpp
	@echo Building $@...
	@$(compiler) -o $@ $(cf) $(threadlib) $(rtlib) $<



run_WaitSet_test: WaitSet_test$(ext)
	@echo running $^...
	@./$^

WaitSet_test$(ext): src/WaitSet_test.cpp
	@echo Building $@...
	@$(compiler) -o $@ $(cf) $(threadlib) $(rtlib) $<



clean:
	@$(remove) debug_test$(ext)
	@$(remove) utils_test$(ext)
	@$(remove) Types_test$(ext)
	@$(remove) WaitSet_test$(ext)
	
include ../../targets.mk

