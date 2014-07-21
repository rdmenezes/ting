$(info entered docs/makefile)

include $(prorab_dir)prorab.mk

#$(info prorab_this_dir = $(prorab_this_dir))

define this_rules
all:: doc

doc: $(prorab_this_dir)/doxygen

$(prorab_this_dir)/doxygen: $(prorab_this_dir)doxygen.cfg
	@echo Building docs...
	@(cd $(prorab_this_dir); doxygen doxygen.cfg)

clean::
	@rm -rf $(prorab_this_dir)doxygen

endef
$(eval $(this_rules))

$(info left docs/makefile)