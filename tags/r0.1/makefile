# Author: Ivan Gagis
# e-mail: igagis@gmail.com

compiler := g++

srcs := src/ting.cpp

libname := libting.so
iface_ver := 1
lib_ver := 0.1

compiler_flags := -Wall -Wno-comment -fstrict-aliasing

$(libname).$(iface_ver).$(lib_ver): $(patsubst %.cpp, %.o, $(srcs) )
	@echo Linking $@...
	@$(compiler) -shared $^ -Wl,-soname,$(libname).$(iface_ver) -o "$@" -s
	ln -s $@ $(libname).$(iface_ver)
	ln -s $(libname).$(iface_ver) $(libname)

%.o: %.cpp
	@echo Compiling $<...
	@$(compiler) -c -fPIC -o "$@" $(compiler_flags) $< 

.PHONY clean:
	rm -f src/*.o
	rm -f $(libname)
	rm -f $(libname).$(iface_ver)
	rm -f $(libname).$(iface_ver).$(lib_ver)