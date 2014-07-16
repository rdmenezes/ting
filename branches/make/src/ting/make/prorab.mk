ifneq ($(prorab_init_included),true)
prorab_init_included := true

#directory of current makefile
prorab-this-dir = $(dir $(lastword $(MAKEFILE_LIST)))


include $(prorab-this-dir)/os.mk

prorab_obj_dir := obj


define prorab-compile-single-cpp
$(prorab_obj_dir)/$(patsubst %.cpp,%.o,$(1)): $(1)
	@echo Compiling $<...
	@mkdir -p $(dir $@)
# -MF option specifies dependency output file name
	@$(CXX) -c -MF "$(patsubst %.o,%.d,$@)" -MD -o "$@" $(CXXFLAGS) $(CPPFLAGS) $(this_сflags) $<
endef


endif

#TODO: clear this_* variables
