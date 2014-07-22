$(info entered makefile)

include prorab.mk


$(eval $(prorab-subdirs-rule))


lib_name := ting



#use exactly this name for the var, so that dh_auto_install could set it if needed
DESTDIR :=
PREFIX := /usr

install:
#install header files
	@for i in $(patsubst src/%,%,$(shell find src/$(lib_name) -type f -name *.hpp)); do \
	    install -D src/$$i $(DESTDIR)$(PREFIX)/include/$$i; \
	done
#install library files
	@install -d $(DESTDIR)$(PREFIX)/lib/
	@install src/lib$(lib_name).* $(DESTDIR)$(PREFIX)/lib/
#install docs
	@install -d $(DESTDIR)$(PREFIX)/share/doc/lib$(lib_name)
	@install docs/doxygen/* $(DESTDIR)$(PREFIX)/share/doc/lib$(lib_name)
#install pkg-config files
	@install -d $(DESTDIR)$(PREFIX)/lib/pkgconfig
	@install pkg-config/*.pc $(DESTDIR)$(PREFIX)/lib/pkgconfig

$(info left makefile)
