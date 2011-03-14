subdirs :=
subdirs += other
subdirs += Socket
subdirs += Array
subdirs += Ptr
#subdirs += Thread
subdirs += SingletonOverSharedLibrary
subdirs += RefOverSharedLibrary



default all clean:
	@$(MAKE) $(subdirs) target=$@

.PHONY: $(subdirs)
$(subdirs):
	@$(MAKE) -C $@ $(target)


