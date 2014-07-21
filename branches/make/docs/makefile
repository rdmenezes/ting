include $(prorab_dir)prorab.mk

$(info prorab_this_dir = $(prorab_this_dir))

all::
	@echo Building docs...
	@doxygen $(prorab_this_dir)doxygen.cfg

clean::
	@rm -rf $(prorab_this_dir)doxygen
