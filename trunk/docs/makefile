$(info entered docs/makefile)


#this should be the first include
ifeq ($(prorab_included),true)
    include $(prorab_dir)prorab.mk
else
    include ../prorab.mk
endif


this_name := ting

$(eval $(prorab-build-doxygen))


$(info left docs/makefile)