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


    .PHONY: clean all

    #define the very first default target
    all::

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
        ifeq ($(prorab_os),windows)
            ifeq ($1,app)
                $(eval prorab_private_name := $(this_name).exe)
            else
                $(eval prorab_private_name := $(this_name).dll)
	    endif
        else
            ifeq ($1,app)
                $(eval prorab_private_name := $(this_name))
            else
                $(eval prorab_private_symbolic_link := $(this_name).so)
                $(eval prorab_private_name := $(prorab_private_symbolic_link).$(this_so_name))
	    endif
        endif


        #default target
        all:: $(prorab_this_dir)$(prorab_private_name)


        ifeq ($1,lib)
            #default target
            all:: $(prorab_this_dir)$(this_name).a


            #static library rule
            $(prorab_this_dir)$(this_name).a: $(addprefix $(prorab_this_dir)$(prorab_obj_dir),$(patsubst %.cpp,%.o,$(this_srcs)))
			@ar cr $$@ $$^


            #addition to "clean" rule
            clean::
			@rm -f $(prorab_this_dir)$(this_name).a


            $(eval prorab_private_lib_ldflags := -shared)

            ifeq ($(prorab_os),windows)
                $(eval prorab_private_lib_ldflags += -s)
            else
                $(eval prorab_private_lib_ldflags += -Wl,-soname,$(prorab_private_name))


                #symbolic link to shared lib rule
                $(prorab_this_dir)$(prorab_private_symbolic_link): $(prorab_this_dir)$(prorab_private_name)
			@echo "Creating symbolic link $$@ -> $$<..."
			@(cd $$(dir $$<); ln -f -s $$(notdir $$<) $$(notdir $$@))


                #addition to "all" rule
                all:: $(prorab_this_dir)$(prorab_private_symbolic_link)


                #addition to "clean" rule
                clean::
			@rm -f $(prorab_this_dir)$(prorab_private_symbolic_link)
            endif
        endif

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
    endef #~prorab-build-rules



    prorab-lib-rules = $(call prorab-build-rules,lib)
    prorab-app-rules = $(call prorab-build-rules,app)



    prorab_private_this_dirs :=

    #include file with correct prorab_this_dir
    define prorab-include
        prorab_private_this_dirs += $(prorab_this_dir)
        prorab_this_dir := $(dir $(prorab_this_dir)$1)
        include $1
        prorab_this_dir := $(lastword $(prorab_private_this_dirs))
	prorab_private_this_dirs := $(wordlist 1,$(call prorab-num,$(call prorab-dec,$(prorab_private_this_dirs))),$(prorab_private_this_dirs))
	
    endef

    #rule for building all makefiles in subdirectories
    define prorab-subdirs-rule
        $(foreach path,$(wildcard $(prorab_this_dir)*/makefile),$(call prorab-include,$(path)))
    endef


endif #~once



#reset this_* variables
$(foreach var,$(filter this_%,$(.VARIABLES)),$(eval $(var) := ))

