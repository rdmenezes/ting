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
