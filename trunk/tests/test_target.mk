test: $(binary_name)
	@echo running $^...
ifeq ($(platform),windows)
	@./$^
else
    ifeq ($(platform),macosx)
	@DYLD_LIBRARY_PATH=../../src ./$^
    else
	@LD_LIBRARY_PATH=../../src ./$^
    endif
endif



gdb: $(binary_name)
	@echo gdb debugging $^...
ifeq ($(platform),windows)
	@gdb ./$^
else
    ifeq ($(platform),macosx)
	@DYLD_LIBRARY_PATH=../../src gdb ./$^
    else
	@LD_LIBRARY_PATH=../../src gdb ./$^
    endif
endif
