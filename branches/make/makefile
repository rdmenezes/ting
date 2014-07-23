$(info entered makefile)

include prorab.mk


$(eval $(prorab-subdirs-rule))


lib_name := ting



install::
#install pkg-config files
	@install -d $(DESTDIR)$(PREFIX)/lib/pkgconfig
	@install pkg-config/*.pc $(DESTDIR)$(PREFIX)/lib/pkgconfig

$(info left makefile)
