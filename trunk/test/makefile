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

#compiler flags
cf := -Wall -I../src
cf += -DDEBUG


test: run_debug_test run_PoolStored_test run_Timer_test run_Thread_test run_Ref_test run_WaitSet_test run_Signal_test



run_debug_test: debug_test$(ext)
	@echo running debug_test...
	@./debug_test$(ext)

debug_test$(ext): src/debug_test.cpp
	@echo Building $@...
	@g++ -o $@ $(cf) $(threadlib) $(rtlib) $<




run_PoolStored_test: PoolStored_test$(ext)
	@echo running PoolStored_Test...
	@./PoolStored_test$(ext)

PoolStored_test$(ext): src/PoolStored_test.cpp
	@echo Building $@...
	@g++ -o $@ $(cf) $(threadlib) $(rtlib) $<



run_Timer_test: Timer_test$(ext)
	@echo running Timer_test...
	@./Timer_test$(ext)

Timer_test$(ext): src/Timer_test.cpp
	@echo Building $@...
	@g++ -o $@ $(cf) $(threadlib) $(rtlib) $<



run_Thread_test: Thread_test$(ext)
	@echo running Thread_test...
	@./Thread_test$(ext)

Thread_test$(ext): src/Thread_test.cpp
	@echo Building $@...
	@g++ -o $@ $(cf) $(threadlib) $(rtlib) $<



run_Ref_test: Ref_test$(ext)
	@echo running Ref_test...
	@./Ref_test$(ext)

Ref_test$(ext): src/Ref_test.cpp
	@echo Building $@...
	@g++ -o $@ $(cf) $<



run_Signal_test: Signal_test$(ext)
	@echo running Signal_test...
	@./Signal_test$(ext)

Signal_test$(ext): src/Signal_test.cpp
	@echo Building $@...
	@g++ -o $@ $(cf) $<



run_WaitSet_test: WaitSet_test$(ext)
	@echo running WaitSet_test...
	@./WaitSet_test$(ext)

WaitSet_test$(ext): src/WaitSet_test.cpp
	@echo Building $@...
	@g++ -o $@ $(cf) $(threadlib) $(rtlib) $<



clean:
	@$(remove) debug_test$(ext)
	@$(remove) PoolStored_test$(ext)
	@$(remove) Timer_test$(ext)
	@$(remove) Thread_test$(ext)
	@$(remove) Ref_test$(ext)
	@$(remove) Signal_test$(ext)
	@$(remove) WaitSet_test$(ext)

