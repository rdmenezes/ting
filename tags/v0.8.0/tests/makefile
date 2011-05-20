subdirs :=
subdirs += other
subdirs += Socket
subdirs += Array
subdirs += Ptr
subdirs += Thread
subdirs += Timer


#Looks like static variables are not shared between dll boundaries on win32,
#so exclude these tests
ifeq ($(platform),win32)

else

subdirs += SingletonOverSharedLibrary
subdirs += RefOverSharedLibrary

endif



include ../targets.mk

$(possible_targets_list):
	@$(MAKE) $(subdirs) target=$@

.PHONY: $(subdirs)
$(subdirs):
	@$(MAKE) -C $@ $(target)


