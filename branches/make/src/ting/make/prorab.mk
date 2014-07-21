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
ifeq ($(prorab_this_dir),./)
    prorab_this_dir :=
endif




ifneq ($(prorab_init_included),true)
    prorab_init_included := true

    .PHONY: clean

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


    #compile rules
    define prorab-build
    all:: $(prorab_this_dir)$(this_name).a $(prorab_this_dir)$(this_name)$(this_extension)

    $(prorab_this_dir)$(prorab_obj_dir)%.o: $(prorab_this_dir)%.cpp
	@echo Compiling $$<...
	@mkdir -p $$(dir $$@)
    # -MF option specifies dependency output file name
	@$(CXX) -c -MF "$$(patsubst %.o,%.d,$$@)" -MD -o "$$@" $(CXXFLAGS) $(CPPFLAGS) $(this_cflags) $$<

    #include rules for header dependencies
    include $(wildcard $(addsuffix *.d,$(dir $(addprefix $(prorab_obj_dir),$(this_srcs)))))

    #symbolic link to shared lib
    ifneq ($(prorab_os),windows)
        $(prorab_this_dir)$(this_name)$(this_extension): $(prorab_this_dir)$(this_name)$(this_extension)$(this_so_name)
	@echo "Creating symbolic link $$@ -> $$<..."
	@ln -f -s $$< $$@
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
	@rm -f $(this_name)$(this_extension)
	@rm -f $(this_name)$(this_extension)$(this_so_name)
	@rm -f $(this_name).a
    endef


endif

#TODO: clear this_* variables
