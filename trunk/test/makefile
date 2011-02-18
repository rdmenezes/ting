subdirs :=
subdirs += other
subdirs += Socket
subdirs += Array
subdirs += Thread



default all clean:
	@$(MAKE) $(subdirs) target=$@

.PHONY: $(subdirs)
$(subdirs):
	@$(MAKE) -C $@ $(target)


