# Prorab build system
# Author: Ivan Gagis <igagis@gmail.com>


#pragma once
ifneq ($(prorab_included),true)
    prorab_included := true


    #for storing included makefiles
    prorab_included_makefiles :=


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
    prorab_this_makefile := $(word $(call prorab-num,$(call prorab-dec,$(MAKEFILE_LIST))),$(MAKEFILE_LIST))
    prorab_this_dir := $(dir $(prorab_this_makefile))


    .PHONY: clean all install


    #define the very first default target
    all:


    #directory of prorab.mk
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



    define prorab-private-app-specific-rules
        $(eval prorab_private_lib_ldflags := )
	
        $(if $(filter windows,$(prorab_os)), \
                $(eval prorab_private_name := $(this_name).exe) \
            , \
                $(eval prorab_private_name := $(this_name)) \
            )
    endef



    define prorab-private-lib-specific-rules-nix-systems
        $(eval prorab_private_symbolic_link := lib$(this_name).so)
        $(eval prorab_private_name := $(prorab_private_symbolic_link).$(this_so_name))
        $(eval prorab_private_lib_ldflags += -Wl,-soname,$(prorab_private_name))

        #symbolic link to shared library rule
        $(prorab_this_dir)$(prorab_private_symbolic_link): $(prorab_this_dir)$(prorab_private_name)
			@echo "Creating symbolic link $$@ -> $$<..."
			@(cd $$(dir $$<); ln -f -s $$(notdir $$<) $$(notdir $$@))

        all: $(prorab_this_dir)$(prorab_private_symbolic_link)

        clean::
			@rm -f $(prorab_this_dir)$(prorab_private_symbolic_link)
    endef


    define prorab-private-lib-specific-rules
        $(eval prorab_private_lib_ldflags := -shared)

        $(if $(filter windows,$(prorab_os)), \
                $(eval prorab_private_name := lib$(this_name).dll) \
                $(eval prorab_private_lib_ldflags += -s) \
            , \
                $(prorab-private-lib-specific-rules-nix-systems) \
            )

        $(eval prorab_private_static_lib_name := lib$(this_name).a)


        all: $(prorab_this_dir)$(prorab_private_static_lib_name)


        #static library rule
        $(prorab_this_dir)$(prorab_private_static_lib_name): $(addprefix $(prorab_this_dir)$(prorab_obj_dir),$(patsubst %.cpp,%.o,$(this_srcs)))
			@echo "Creating static library $$@..."
			@ar cr $$@ $$^


        clean::
			@rm -f $(prorab_this_dir)$(prorab_private_static_lib_name)

    endef


    define prorab-private-common-rules
        #default target
        all: $(prorab_this_dir)$(prorab_private_name)


        #compile pattern rule
        $(prorab_this_dir)$(prorab_obj_dir)%.o: $(prorab_this_dir)%.cpp
		@echo Compiling $$<...
		@mkdir -p $$(dir $$@)
		@$$(CXX) -c -MF "$$(patsubst %.o,%.d,$$@)" -MD -o "$$@" $(CXXFLAGS) $(CPPFLAGS) $(this_cflags) $$<

        #include rules for header dependencies
        include $(wildcard $(addsuffix *.d,$(dir $(addprefix $(prorab_this_dir)$(prorab_obj_dir),$(this_srcs)))))

        #link rule
        $(prorab_this_dir)$(prorab_private_name): $(addprefix $(prorab_this_dir)$(prorab_obj_dir),$(patsubst %.cpp,%.o,$(this_srcs)))
		@echo Linking $$@...
		@$$(CXX) $$^ -o "$$@" $(this_ldlibs) $(this_ldflags) $(LDLIBS) $(LDFLAGS) $(prorab_private_lib_ldflags)


        #clean rule
        clean::
		@rm -rf $(prorab_this_dir)$(prorab_obj_dir)
		@rm -f $(prorab_this_dir)$(prorab_private_name)
    endef


    define prorab-lib-rules
        $(prorab-private-lib-specific-rules)
        $(prorab-private-common-rules)
    endef


    define prorab-app-rules
        $(prorab-private-app-specific-rules)
        $(prorab-private-common-rules)
    endef




    define prorab-include
        $(if $(filter $1,$(prorab_included_makefiles)), \
            , \
                $(eval prorab_included_makefiles += $1) \
                $(call prorab-private-include,$1) \
            )
    endef


    #for storing previous prorab_this_makefile when including other makefiles
    prorab_private_this_makefiles :=

    #include file with correct prorab_this_dir
    define prorab-private-include
        prorab_private_this_makefiles += $$(prorab_this_dir)
        prorab_this_makefile := $1
        prorab_this_dir := $$(dir $$(prorab_this_makefile))
        include $1
        prorab_this_makefile := $$(lastword $$(prorab_private_this_makefiles))
        prorab_this_dir := $$(dir $$(prorab_this_makefile))
        prorab_private_this_makefiles := $$(wordlist 1,$$(call prorab-num,$$(call prorab-dec,$$(prorab_private_this_makefiles))),$$(prorab_private_this_makefiles))

    endef

    #rule for building all makefiles in subdirectories
    define prorab-subdirs-rule
        $(foreach path,$(wildcard $(prorab_this_dir)*/makefile),$(call prorab-include,$(path)))
    endef


    

endif #~once


$(if $(filter $(prorab_this_makefile),$(prorab_included_makefiles)), \
        \
    , \
        $(eval prorab_included_makefiles += $(prorab_this_makefile)) \
    )

#$(info $(prorab_included_makefiles))

#reset this_* variables
$(foreach var,$(filter this_%,$(.VARIABLES)),$(eval $(var) := ))

