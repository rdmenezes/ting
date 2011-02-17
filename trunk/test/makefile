subdirs :=
subdirs += other
subdirs += Socket
subdirs += Array



default all clean:
	@$(MAKE) $(subdirs) target=$@

.PHONY: $(subdirs)
$(subdirs):
	@$(MAKE) -C $@ $(target)


