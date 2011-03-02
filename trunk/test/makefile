subdirs :=
subdirs += other
subdirs += Socket
subdirs += Array
subdirs += Thread
subdirs += SingletonOverSharedLibrary



default all clean:
	@$(MAKE) $(subdirs) target=$@

.PHONY: $(subdirs)
$(subdirs):
	@$(MAKE) -C $@ $(target)


