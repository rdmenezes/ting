$(info entered makefile)

include prorab.mk


$(eval $(prorab-build-subdirs))

this_soname := $(shell cat $(prorab_this_dir)src/soname.txt)

install::
#install pkg-config files
	@install -d $(DESTDIR)$(PREFIX)/lib/pkgconfig
	@install pkg-config/*.pc $(DESTDIR)$(PREFIX)/lib/pkgconfig
#install prorab.mk
	@install -d $(DESTDIR)$(PREFIX)/include
	@install prorab.mk $(DESTDIR)$(PREFIX)/include


$(eval $(prorab-build-deb))

$(info left makefile)
