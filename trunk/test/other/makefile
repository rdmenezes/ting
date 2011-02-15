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
ifeq ($(platform),win32)
threadlib := 
rtlib :=
endif

compiler := g++

#compiler flags
cf := -Wall -I../../src
cf += -DDEBUG


default: run_debug_test run_utils_test run_PoolStored_test run_Timer_test run_Buffer_test run_Types_test run_Thread_test run_Ref_test run_WaitSet_test run_Signal_test



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




run_PoolStored_test: PoolStored_test$(ext)
	@echo running $^...
	@./$^

PoolStored_test$(ext): src/PoolStored_test.cpp
	@echo Building $@...
	@$(compiler) -o $@ $(cf) $(threadlib) $(rtlib) $<



run_Timer_test: Timer_test$(ext)
	@echo running $^...
	@./$^

Timer_test$(ext): src/Timer_test.cpp
	@echo Building $@...
	@$(compiler) -o $@ $(cf) $(threadlib) $(rtlib) $<



run_Buffer_test: Buffer_test$(ext)
	@echo running $^...
	@./$^

Buffer_test$(ext): src/Buffer_test.cpp
	@echo Building $@...
	@$(compiler) -o $@ $(cf) $(threadlib) $(rtlib) $<



run_Types_test: Types_test$(ext)
	@echo running $^...
	@./$^

Types_test$(ext): src/Types_test.cpp
	@echo Building $@...
	@$(compiler) -o $@ $(cf) $(threadlib) $(rtlib) $<



run_Thread_test: Thread_test$(ext)
	@echo running $^...
	@./$^

Thread_test$(ext): src/Thread_test.cpp
	@echo Building $@...
	@$(compiler) -o $@ $(cf) $(threadlib) $(rtlib) $<



run_Ref_test: Ref_test$(ext)
	@echo running $^...
	@./$^

Ref_test$(ext): src/Ref_test.cpp
	@echo Building $@...
	@$(compiler) -o $@ $(cf) $<



run_Signal_test: Signal_test$(ext)
	@echo running $^...
	@./$^

Signal_test$(ext): src/Signal_test.cpp
	@echo Building $@...
	@$(compiler) -o $@ $(cf) $<



run_WaitSet_test: WaitSet_test$(ext)
	@echo running $^...
	@./$^

WaitSet_test$(ext): src/WaitSet_test.cpp
	@echo Building $@...
	@$(compiler) -o $@ $(cf) $(threadlib) $(rtlib) $<



clean:
	@$(remove) debug_test$(ext)
	@$(remove) utils_test$(ext)
	@$(remove) PoolStored_test$(ext)
	@$(remove) Timer_test$(ext)
	@$(remove) Buffer_test$(ext)
	@$(remove) Types_test$(ext)
	@$(remove) Thread_test$(ext)
	@$(remove) Ref_test$(ext)
	@$(remove) Signal_test$(ext)
	@$(remove) WaitSet_test$(ext)

