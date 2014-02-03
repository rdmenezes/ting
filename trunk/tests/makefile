include ../platform.mk

subdirs :=
subdirs += Array
subdirs += atomic
subdirs += Buffer
subdirs += debug
subdirs += FSFile
subdirs += math
subdirs += MemoryFile
subdirs += net
subdirs += PoolStored
subdirs += Ptr
subdirs += Ref
subdirs += RefOverSharedLibrary
subdirs += Signal
subdirs += Thread
subdirs += timer
subdirs += types
subdirs += utf8
subdirs += util
subdirs += WaitSet


#Looks like static variables are not shared between dll boundaries on win32,
#so exclude these tests
ifeq ($(platform),windows)

else

subdirs += SingletonOverSharedLibrary

endif



include ../targets.mk

$(possible_targets_list):
	@$(MAKE) $(subdirs) target=$@

.PHONY: $(subdirs)
$(subdirs):
	@$(MAKE) -C $@ $(target)


