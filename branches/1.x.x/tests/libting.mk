ifeq ($(platform),win32)

libting := ../../src/libting.a

else

libting := ../../src/libting.so

endif

$(libting):
	$(MAKE) -C ../../src
