subdirs :=
subdirs += other
subdirs += Socket
subdirs += Array
subdirs += Thread
subdirs += SingletonOverSharedLibrary
subdirs += RefOverSharedLibrary



default all clean:
	@$(MAKE) $(subdirs) target=$@

.PHONY: $(subdirs)
$(subdirs):
	@$(MAKE) -C $@ $(target)


