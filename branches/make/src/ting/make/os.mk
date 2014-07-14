ifeq ($(os),) #if os not defined or empty



operating_system := $(shell uname)
ifeq ($(patsubst MINGW%,MINGW,$(operating_system)), MINGW)
    os := windows
else ifeq ($(operating_system), Darwin)
    os := macosx
else
    os := linux
endif



endif
