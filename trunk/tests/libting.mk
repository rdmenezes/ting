ifeq ($(platform),windows)

    libting := ../../src/libting.a

else

    libting := ../../src/libting.so

endif

$(libting):
	$(MAKE) -C ../../src
