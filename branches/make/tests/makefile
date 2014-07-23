$(info entered tests/makefile)

#this should be the first include
ifeq ($(prorab_included),true)
    include $(prorab_dir)prorab.mk
else
    include ../prorab.mk
endif


$(eval $(prorab-build-subdirs))


$(info left tests/makefile)