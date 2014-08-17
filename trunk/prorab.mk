# Prorab build system
# Copyright Ivan Gagis <igagis@gmail.com>, 2014


#pragma once
ifneq ($(prorab_included),true)
    prorab_included := true


    #for storing list of included makefiles
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

    #define local variables used by prorab
    this_name :=
    this_so_name :=
    this_cflags :=
    this_ldflags :=
    this_ldlibs :=
    this_srcs :=
    this_objs :=
    


    .PHONY: clean all install distclean


    #define the very first default target
    all:

    #define distclean target which does same as clean. This is to make some older versions of debhelper happy.
    distclean: clean


    #directory of prorab.mk
    prorab_dir := $(dir $(lastword $(MAKEFILE_LIST)))

    #initialize standard vars for "install" target
    ifeq ($(PREFIX),) #PREFIX is environment variable, but if it is not set, then set default value
        PREFIX := /usr/local
    endif

    #Detect operating system
    prorab_private_os := $(shell uname)
    ifeq ($(patsubst MINGW%,MINGW,$(prorab_private_os)), MINGW)
        prorab_os := windows
    else ifeq ($(prorab_private_os), Darwin)
        prorab_os := macosx
    else
        prorab_os := linux
    endif

    #set library extension
    ifeq ($(prorab_os), windows)
        prorab_lib_extension := .dll
    else ifeq ($(prorab_os), macosx)
        prorab_lib_extension := .dylib
    else
        prorab_lib_extension := .so
    endif



    prorab_obj_dir := obj/


    prorab_echo := @


    define prorab-private-app-specific-rules
        $(eval prorab_private_ldflags := )

        $(if $(filter windows,$(prorab_os)), \
                $(eval prorab_private_name := $(abspath $(prorab_this_dir)$(this_name).exe)) \
            , \
                $(eval prorab_private_name := $(abspath $(prorab_this_dir)$(this_name))) \
            )

        $(eval prorab_this_name := $(prorab_private_name))

        install:: $(prorab_private_name)
		$(prorab_echo)install -d $(DESTDIR)$(PREFIX)/bin/
		$(prorab_echo)install $(prorab_private_name) $(DESTDIR)$(PREFIX)/bin/
    endef



    define prorab-private-lib-specific-rules-nix-systems
        $(if $(filter macosx,$(prorab_os)), \
                $(eval prorab_private_symbolic_link := $(abspath $(prorab_this_dir)lib$(this_name)$(prorab_lib_extension))) \
                $(eval prorab_private_name := $(abspath $(prorab_this_dir)lib$(this_name).$(this_so_name)$(prorab_lib_extension))) \
                $(eval prorab_private_ldflags += -dynamiclib -Wl,-install_name,$(prorab_private_name),-headerpad_max_install_names,-undefined,dynamic_lookup,-compatibility_version,1.0,-current_version,1.0) \
            ,\
                $(eval prorab_private_symbolic_link := $(abspath $(prorab_this_dir)lib$(this_name)$(prorab_lib_extension))) \
                $(eval prorab_private_name := $(prorab_private_symbolic_link).$(this_so_name)) \
                $(eval prorab_private_ldflags := -shared -Wl,-soname,$(notdir $(prorab_private_name))) \
            )

        $(eval prorab_this_name := $(prorab_private_symbolic_link))

        #symbolic link to shared library rule
        $(prorab_private_symbolic_link): $(prorab_private_name)
			@echo "Creating symbolic link $$(notdir $$@) -> $$(notdir $$<)..."
			$(prorab_echo)(cd $$(dir $$<); ln -f -s $$(notdir $$<) $$(notdir $$@))

        all: $(prorab_private_symbolic_link)

        install:: $(prorab_private_symbolic_link)
		@install -d $(DESTDIR)$(PREFIX)/lib/
		$(prorab_echo)(cd $(DESTDIR)$(PREFIX)/lib/; ln -f -s $(notdir $(prorab_private_name)) $(notdir $(prorab_private_symbolic_link)))

        clean::
		$(prorab_echo)rm -f $(prorab_private_symbolic_link)
    endef


    define prorab-private-lib-specific-rules
        $(if $(filter windows,$(prorab_os)), \
                $(eval prorab_private_name := $(abspath $(prorab_this_dir)lib$(this_name)$(prorab_lib_extension))) \
                $(eval prorab_private_ldflags := -shared -s) \
                $(eval prorab_this_name := $(prorab_private_name)) \
            , \
                $(prorab-private-lib-specific-rules-nix-systems) \
            )

        $(eval prorab_this_staticlib := $(abspath $(prorab_this_dir)lib$(this_name).a))


        all: $(prorab_this_staticlib)


        #static library rule
        $(prorab_this_staticlib): $(addprefix $(prorab_this_dir)$(prorab_obj_dir),$(patsubst %.cpp,%.o,$(this_srcs)))
			@echo "Creating static library $$(notdir $$@)..."
			$(prorab_echo)ar cr $$@ $$^ $(this_objs)


        clean::
		$(prorab_echo)rm -f $(prorab_this_staticlib)

        install:: $(prorab_this_staticlib) $(prorab_private_name)
		$(prorab_echo)for i in $(patsubst $(prorab_this_dir)%,%,$(shell find $(prorab_this_dir) -type f -name "*.hpp")); do \
		    install -d $(DESTDIR)$(PREFIX)/include/$$$${i%/*}; \
		    install $(prorab_this_dir)$$$$i $(DESTDIR)$(PREFIX)/include/$$$$i; \
		done
		$(prorab_echo)install -d $(DESTDIR)$(PREFIX)/lib/
		$(prorab_echo)install $(prorab_this_staticlib) $(DESTDIR)$(PREFIX)/lib/
		$(prorab_echo)install $(prorab_private_name) $(DESTDIR)$(PREFIX)/lib/
		$(if $(filter macosx,$(prorab_os)), \
		        $(prorab_echo)install_name_tool -id "$(PREFIX)/lib/$(notdir $(prorab_private_name))" $(DESTDIR)$(PREFIX)/lib/$(notdir $(prorab_private_name)) \
		    )
    endef


    define prorab-private-common-rules

        all: $(prorab_private_name)

        $(eval prorab_this_objs := $(addprefix $(prorab_this_dir)$(prorab_obj_dir),$(patsubst %.cpp,%.o,$(this_srcs))))

        #compile static pattern rule
        $(prorab_this_objs): $(prorab_this_dir)$(prorab_obj_dir)%.o: $(prorab_this_dir)%.cpp $(prorab_this_dir)makefile
		@echo Compiling $$<...
		$(prorab_echo)mkdir -p $$(dir $$@)
		$(prorab_echo)$$(CXX) -c -MF "$$(patsubst %.o,%.d,$$@)" -MD -o "$$@" $(CXXFLAGS) $(CPPFLAGS) $(this_cflags) $$<

        #include rules for header dependencies
        include $(wildcard $(addsuffix *.d,$(dir $(addprefix $(prorab_this_dir)$(prorab_obj_dir),$(this_srcs)))))

        #link rule
        $(prorab_private_name): $(prorab_this_objs) $(prorab_this_dir)makefile
		@echo Linking $$@...
		$(prorab_echo)$$(CXX) $(prorab_this_objs) $(this_objs) -o "$$@" $(this_ldlibs) $(this_ldflags) $(LDLIBS) $(LDFLAGS) $(prorab_private_ldflags)

        #clean rule
        clean::
		$(prorab_echo)rm -rf $(prorab_this_dir)$(prorab_obj_dir)
		$(prorab_echo)rm -f $(prorab_private_name)
    endef


    define prorab-build-lib
        $(prorab-private-lib-specific-rules)
        $(prorab-private-common-rules)
    endef


    define prorab-build-app
        $(prorab-private-app-specific-rules)
        $(prorab-private-common-rules)
    endef




    define prorab-include
        $(if $(filter $(abspath $1),$(prorab_included_makefiles)), \
            , \
                $(eval prorab_included_makefiles += $(abspath $1)) \
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

    #include all makefiles in subdirectories
    define prorab-build-subdirs
        $(foreach path,$(wildcard $(prorab_this_dir)*/makefile),$(call prorab-include,$(path)))
    endef


    prorab-clear-this-vars = $(foreach var,$(filter this_%,$(.VARIABLES)),$(eval $(var) := ))
    


    define prorab-build-doxygen
        all: doc

        doc:: $(prorab_this_dir)doxygen

        $(prorab_this_dir)doxygen: $(prorab_this_dir)doxygen.cfg
		@echo Building docs...
		@(cd $(prorab_this_dir); doxygen doxygen.cfg || true)

        clean::
		@rm -rf $(prorab_this_dir)doxygen

        install::
		@install -d $(DESTDIR)$(PREFIX)/share/doc/$(this_name)
		@install $(prorab_this_dir)doxygen/* $(DESTDIR)$(PREFIX)/share/doc/$(this_name) || true #ignore error, not all systems have doxygen

    endef


endif #~once


$(if $(filter $(prorab_this_makefile),$(prorab_included_makefiles)), \
        \
    , \
        $(eval prorab_included_makefiles += $(abspath $(prorab_this_makefile))) \
    )

#$(info $(prorab_included_makefiles))


$(prorab-clear-this-vars)
