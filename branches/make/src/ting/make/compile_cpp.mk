#TODO: check that inited

$(ting_obj_dir)/%.o:%.cpp
	@echo Compiling $<...
	@mkdir -p $(dir $@)
# -MF option specifies dependency output file name
	@$(CXX) -c -MF "$(patsubst %.o,%.d,$@)" -MD -o "$@" $(CXXFLAGS) $(CPPFLAGS) $(this_сflags) $<

include $(wildcard $(addsuffix /*.d,$(dir $(addprefix $(ting_obj_dir)/,$(this_srcs)))))
