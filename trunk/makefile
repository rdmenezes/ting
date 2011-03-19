subdirs :=
subdirs += tests
subdirs += docs



default all clean test:
	@$(MAKE) $(subdirs) target=$@

.PHONY: $(subdirs)
$(subdirs):
	@$(MAKE) -C $@ $(target)


