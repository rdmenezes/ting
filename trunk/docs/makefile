$(info entered docs/makefile)


#this should be the first include
ifeq ($(prorab_included),true)
    include $(prorab_dir)prorab.mk
else
    include ../prorab.mk
endif


#$(info prorab_this_dir = $(prorab_this_dir))

define this_rules
all: doc

doc: $(prorab_this_dir)doxygen

$(prorab_this_dir)doxygen: $(prorab_this_dir)doxygen.cfg
	@echo Building docs...
	@(cd $(prorab_this_dir); doxygen doxygen.cfg || true)

clean::
	@rm -rf $(prorab_this_dir)doxygen

install::
	@install -d $(DESTDIR)$(PREFIX)/share/doc/libting
	@install $(prorab_this_dir)doxygen/* $(DESTDIR)$(PREFIX)/share/doc/libting || true

endef
#$(info $(this_rules))
$(eval $(this_rules))

$(info left docs/makefile)