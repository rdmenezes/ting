#define arithmetic functions
ifneq ($(prorab_arithmetic_functions_defined),true)
    prorab_arithmetic_functions_defined := true

    prorab-num = $(words $1)
    prorab-add = $1 $2
    prarab-inc = x $1
    prorab-dec = $(wordlist 2,$(words $1),$1)

endif




#define this directory for parent makefile
prorab_this_dir := $(dir $(word $(call prorab-num,$(call prorab-dec,$(MAKEFILE_LIST))),$(MAKEFILE_LIST)))





ifneq ($(prorab_init_included),true)
    prorab_init_included := true

    #directory of current makefile
    prorab_dir := $(dir $(lastword $(MAKEFILE_LIST)))


    #Detect operating system
    prorab_operating_system := $(shell uname)
    ifeq ($(patsubst MINGW%,MINGW,$(prorab_operating_system)), MINGW)
        prorab_os := windows
    else ifeq ($(prorab_operating_system), Darwin)
        prorab_os := macosx
    else
        prorab_os := linux
    endif



    prorab_obj_dir := obj/


    define prorab-compile-single-cpp
    $(prorab_this_dir)$(prorab_obj_dir)$(patsubst %.cpp,%.o,$(1)): $(prorab_this_dir)$(1)
	@echo Compiling $<...
	mkdir -p $(dir $@)
    # -MF option specifies dependency output file name
	$(CXX) -c -MF "$(patsubst %.o,%.d,$@)" -MD -o "$@" $(CXXFLAGS) $(CPPFLAGS) $(this_сflags) $<
    endef

    define prorab-compile-cpp
    $(foreach src,$(this_srcs),$(call prorab-compile-single-cpp,$(src)))

    include $(wildcard $(addsuffix /*.d,$(dir $(addprefix $(prorab_obj_dir)/,$(this_srcs)))))
    endef

endif

#TODO: clear this_* variables
