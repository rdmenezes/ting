test:: $(prorab_this_dir)$(this_name)
	@echo running $^...
ifeq ($(prorab_os),windows)
	@./$^
else
    ifeq ($(prorab_os),macosx)
	@DYLD_LIBRARY_PATH=../../src ./$^
    else
	@LD_LIBRARY_PATH=../../src ./$^
    endif
endif



gdb:: $(prorab_this_dir)$(this_name)
	@echo gdb debugging $^...
ifeq ($(prorab_os),windows)
	@gdb ./$^
else
    ifeq ($(prorab_os),macosx)
	@DYLD_LIBRARY_PATH=../../src gdb ./$^
    else
	@LD_LIBRARY_PATH=../../src gdb ./$^
    endif
endif
