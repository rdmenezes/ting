platform := linux
operating_system := $(shell uname)
ifeq ($(operating_system), Msys)
    platform := windows
endif
ifeq ($(operating_system), Darwin)
    platform := macosx
endif
