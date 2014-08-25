$(info entered makefile)

include prorab.mk


$(eval $(prorab-build-subdirs))


install::
#install pkg-config files
	@install -d $(DESTDIR)$(PREFIX)/lib/pkgconfig
	@install pkg-config/*.pc $(DESTDIR)$(PREFIX)/lib/pkgconfig
#install prorab.mk
	@install -d $(DESTDIR)$(PREFIX)/include
	@install prorab.mk $(DESTDIR)$(PREFIX)/include




$(prorab-clear-this-vars)

this_soname_dependency := $(prorab_this_dir)src/soname.txt

this_soname := $(shell cat $(this_soname_dependency))


$(eval $(prorab-build-deb))




$(info left makefile)
