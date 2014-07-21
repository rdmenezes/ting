# Prorab build system
# Author: Ivan Gagis <igagis@gmail.com>


#pragma once
ifneq ($(prorab_included),true)
    prorab_included := true


    #check if running minimal supported GNU make version
    prorab_min_gnumake_version := 3.81
    ifeq ($(filter $(prorab_min_gnumake_version),$(firstword $(sort $(MAKE_VERSION) $(prorab_min_gnumake_version)))),)
        $(error GNU make $(prorab_min_gnumake_version) or higher is needed, but found only $(MAKE_VERSION))
    endif


    #check that prorab.mk is the first file included
    ifneq ($(words $(MAKEFILE_LIST)),2)
        $(error prorab.mk is not a first include in the makefile, include prorab.mk should be the very first thing done in the makefile.)
    endif


    #define arithmetic functions
    prorab-num = $(words $1)
    prorab-add = $1 $2
    prarab-inc = x $1
    prorab-dec = $(wordlist 2,$(words $1),$1)


    #define this directory for parent makefile
    prorab_this_dir := $(dir $(word $(call prorab-num,$(call prorab-dec,$(MAKEFILE_LIST))),$(MAKEFILE_LIST)))
    ifeq ($(prorab_this_dir),./)
        prorab_this_dir :=
    endif


    .PHONY: clean all


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


    #build rule
    define prorab-build-rules
    all:: $(prorab_this_dir)$(this_name).a $(prorab_this_dir)$(this_name)$(this_extension)

    $(prorab_this_dir)$(prorab_obj_dir)%.o: $(prorab_this_dir)%.cpp
	@echo Compiling $$<...
	@mkdir -p $$(dir $$@)
    # -MF option specifies dependency output file name
	@$(CXX) -c -MF "$$(patsubst %.o,%.d,$$@)" -MD -o "$$@" $(CXXFLAGS) $(CPPFLAGS) $(this_cflags) $$<

    #include rules for header dependencies
    include $(wildcard $(addsuffix *.d,$(dir $(addprefix $(prorab_this_dir)$(prorab_obj_dir),$(this_srcs)))))

    #symbolic link to shared lib
    ifneq ($(prorab_os),windows)
        $(prorab_this_dir)$(this_name)$(this_extension): $(prorab_this_dir)$(this_name)$(this_extension)$(this_so_name)
	@echo "Creating symbolic link $$@ -> $$<..."
	@(cd $$(dir $$<); ln -f -s $$(notdir $$<) $$(notdir $$@))
    endif

    #static library rule
    $(prorab_this_dir)$(this_name).a: $(addprefix $(prorab_this_dir)$(prorab_obj_dir),$(patsubst %.cpp,%.o,$(this_srcs)))
	@ar cr $$@ $$^

    #link rule
    $(prorab_this_dir)$(this_name)$(this_extension)$(this_so_name): $(addprefix $(prorab_this_dir)$(prorab_obj_dir),$(patsubst %.cpp,%.o,$(this_srcs)))
	@echo Linking $$@...
	@$(CXX) $$^ -o "$$@" $(this_ldlibs) $(this_ldflags) $(LDLIBS) $(LDFLAGS)

    #clean rule
    clean::
	@rm -rf $(prorab_this_dir)$(prorab_obj_dir)
	@rm -f $(prorab_this_dir)$(this_name)$(this_extension)
	@rm -f $(prorab_this_dir)$(this_name)$(this_extension)$(this_so_name)
	@rm -f $(prorab_this_dir)$(this_name).a
    endef




    prorab_private_this_dirs :=

    define prorab-include
        prorab_private_this_dirs += $(prorab_this_dir)
        prorab_this_dir := $(dir $(prorab_this_dir)$1)
        include $1
        prorab_this_dir := $(lastword $(prorab_private_this_dirs))
	prorab_private_this_dirs := $(wordlist 1,$(call prorab-num,$(call prorab-dec,$(prorab_private_this_dirs))),$(prorab_private_this_dirs))
	
    endef


    define prorab-subdirs-rule
        $(foreach path,$(wildcard $(prorab_this_dir)*/makefile),$(call prorab-include,$(path)))
    endef


    #new line character
    define prorab_newline


    endef


endif



#reset this_* variables
$(eval $(foreach var,$(filter this_%,$(.VARIABLES)),$(var) := $(prorab_newline)))

