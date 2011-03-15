subdirs :=
subdirs += other
subdirs += Socket
subdirs += Array
subdirs += Ptr
subdirs += Thread


#Looks like static variables are not shared between dll boundaries on win32,
#so exclude these tests
ifeq ($(platform),win32)

else

subdirs += SingletonOverSharedLibrary
subdirs += RefOverSharedLibrary

endif




default all clean:
	@$(MAKE) $(subdirs) target=$@

.PHONY: $(subdirs)
$(subdirs):
	@$(MAKE) -C $@ $(target)


