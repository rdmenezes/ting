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


test: PoolStored_test$(ext) Timer_test$(ext)
	@echo running PoolStored_Test...
	@./PoolStored_test$(ext)
	@echo running Timer_test...
	@./Timer_test$(ext)


PoolStored_test$(ext): src/PoolStored_test.cpp
	@echo Building $@...
	@g++ -o $@ $(cf) $(threadlib) $(rtlib) $<

Timer_test$(ext): src/Timer_test.cpp
	@echo Building $@...
	@g++ -o $@ $(cf) $(threadlib) $(rtlib) $<

clean:
	@$(remove) PoolStored_test$(ext)
	@$(remove) Timer_test$(ext)
