$(info entered src/makefile)

#this should be the first include
ifeq ($(prorab_included),true)
    include $(prorab_dir)prorab.mk
else
    include ../prorab.mk
endif


debug := false


this_name := ting

this_so_name := 3



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

ifeq ($(debug),true)
    this_cflags += -DDEBUG
endif




this_cflags += -Wall #enable all warnings
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



ifeq ($(prorab_os),linux)
    this_cflags += -fPIC # Since we are building shared library, we need Position-Independend Code
endif





#=======================
#  Include directories
#=======================
ifeq ($(prorab_os),linux)
    this_cflags +=
else ifeq ($(prorab_os),windows)
    this_cflags +=
endif



#============================
#  Libs and lib directories
#============================
ifeq ($(prorab_os),linux)
    this_ldlibs := -lpthread -lrt
else ifeq ($(prorab_os),windows)
    this_ldlibs := -lws2_32
endif





$(eval $(prorab-lib-rules))


$(info left src/makefile)
