$(info entered makefile)

include prorab.mk


$(eval $(prorab-subdirs-rule))


lib_name := ting



install::
#install pkg-config files
	@install -d $(DESTDIR)$(PREFIX)/lib/pkgconfig
	@install pkg-config/*.pc $(DESTDIR)$(PREFIX)/lib/pkgconfig
#install prorab.mk
	@install -d $(DESTDIR)$(PREFIX)/include
	@install prorab.mk $(DESTDIR)$(PREFIX)/include

$(info left makefile)
