# Author: Ivan Gagis <igagis@gmail.com>

#==============================================================================
#==============================================================================
#==============================================================================
#                        Project configuration part
#             change these strings to configure project building

#Known platforms are:
#    linux
#    maemo_linux
#    win32
#    macosx
include ting/make/prorab.mk

debug := false

name := libting

so_name := 3

ifeq ($(os), linux)
    name := $(name).so
endif
ifeq ($(os), macosx)
    name := $(name).so
endif
ifeq ($(os), windows)
    name := $(name).dll
endif



#Sources
this_srcs :=
this_srcs += ting/fs/BufferFile.cpp
this_srcs += ting/fs/File.cpp
this_srcs += ting/fs/FSFile.cpp
this_srcs += ting/fs/MemoryFile.cpp
this_srcs += ting/mt/Mutex.cpp
this_srcs += ting/mt/MsgThread.cpp
this_srcs += ting/mt/Queue.cpp
this_srcs += ting/mt/Semaphore.cpp
this_srcs += ting/mt/Thread.cpp
this_srcs += ting/net/HostNameResolver.cpp
this_srcs += ting/net/IPAddress.cpp
this_srcs += ting/net/Lib.cpp
this_srcs += ting/net/Socket.cpp
this_srcs += ting/net/TCPServerSocket.cpp
this_srcs += ting/net/TCPSocket.cpp
this_srcs += ting/net/UDPSocket.cpp
this_srcs += ting/Ref.cpp
this_srcs += ting/timer.cpp
this_srcs += ting/WaitSet.cpp



this_cflags :=

ifeq ($(debug), true)
    this_cflags += -DDEBUG
endif

ifeq ($(os), maemo_linux)
    this_cflags += -DM_MAEMO
endif



this_cflags := -Wall #enable all warnings
this_cflags += -Wno-comment #no warnings on nested comments
this_cflags += -funsigned-char #the 'char' type is unsigned
this_cflags += -Wnon-virtual-dtor #warn if base class has non-virtual destructor
this_cflags += -fno-operator-names #do not treat 'and', 'bitand','bitor','compl','not','or','xor' as keywords
this_cflags += -Werror #treat warnings as errors
this_cflags += -Wfatal-errors # stop on first error encountered
this_cflags += -fstrict-aliasing #strict aliasing (in order to comply with the c++ standard more strictly)
this_cflags += -g #include debugging symbols
                #-P -E #-Wreorder
                #-O3 -funroll-loops -fomit-frame-pointer
#compiler_flags += -std=c++0x



ifeq ($(os), linux)
    this_cflags += -fPIC # Since we are building shared library, we need Position-Independend Code
endif
ifeq ($(os), maemo_linux)
    this_cflags += -fPIC # Since we are building shared library, we need Position-Independend Code
endif


linker_flags := -shared

ifeq ($(os), linux)
    linker_flags += -Wl,-soname,$(name).$(so_name)
endif
ifeq ($(os), windows)
    linker_flags += -s
endif



#=======================
#  Include directories
#=======================
ifeq ($(os), maemo_linux)
    this_cflags +=
endif
ifeq ($(os), linux)
    this_cflags +=
endif
ifeq ($(os), windows)
    this_cflags +=
endif



#============================
#  Libs and lib directories
#============================
ifeq ($(os), maemo_linux)
    libs := -lpthread -lrt
endif
ifeq ($(os), linux)
    libs := -lpthread -lrt
endif
ifeq ($(os), windows)
    libs := -lws2_32
endif



#                     __
#                    /  \__/  end of configuration part
#==============================================================================


obj_dir := obj/1/2
compiler := g++


#==============================================================================
#=============================TARGETS==========================================
#==============================================================================

#==========================
#=project default target=
default: create-obj-dir $(name)

create-obj-dir:
	@mkdir -p $(obj_dir)



#find all .cpp files and get an .o file name for it to get dependancies for this target
$(name): $(addprefix $(obj_dir)/,$(patsubst %.cpp,%.o,$(this_srcs)))
	@echo Linking $@...
ifeq ($(os), windows)
	@ar cr $(patsubst %.dll,%.a,$@) $^
	@$(compiler) $^ -o "$@" $(libs) $(linker_flags)
else
	@ar cr $(patsubst %.so,%.a,$@) $^
	@$(compiler) $^ -o "$@".$(so_name) $(libs) $(linker_flags)
	@ln -f -s $@.$(so_name) $@
endif


$(call prorab-compile-single-cpp,ting/WaitSet.cpp)



#==================
#=build all target=
all: clean proj


test:

#==============
#=clean target=
#delete all objects and executables

#it is an abstract target (not a file), declare as .PHONY
.PHONY: clean
clean:
ifeq ($(os),windows)
	@rm -f $(name)
	@rm -f $(patsubst %.dll,%.a,$(name))
	@rm -rf $(dir $(obj_dir))
else
	@rm -f $(name)
	@rm -f $(patsubst %.so,%.a,$(name))
	@rm -f $(name).$(so_name)
	@rm -rf $(dir $(obj_dir))
endif

include ../targets.mk
