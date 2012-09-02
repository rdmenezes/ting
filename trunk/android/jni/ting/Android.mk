LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

#TARGET_BUILD_TYPE := debug

#LOCAL_ARM_MODE   := arm #arm or thumb

LOCAL_MODULE    := ting

LOCAL_SRC_FILES :=
LOCAL_SRC_FILES += ting/fs/File.cpp
LOCAL_SRC_FILES += ting/fs/FSFile.cpp
LOCAL_SRC_FILES += ting/mt/MsgThread.cpp
LOCAL_SRC_FILES += ting/mt/Mutex.cpp
LOCAL_SRC_FILES += ting/mt/Queue.cpp
LOCAL_SRC_FILES += ting/mt/Semaphore.cpp
LOCAL_SRC_FILES += ting/mt/Thread.cpp
LOCAL_SRC_FILES += ting/net/HostNameResolver.cpp
LOCAL_SRC_FILES += ting/net/IPAddress.cpp
LOCAL_SRC_FILES += ting/net/Lib.cpp
LOCAL_SRC_FILES += ting/net/Socket.cpp
LOCAL_SRC_FILES += ting/net/TCPServerSocket.cpp
LOCAL_SRC_FILES += ting/net/TCPSocket.cpp
LOCAL_SRC_FILES += ting/net/UDPSocket.cpp
LOCAL_SRC_FILES += ting/Ref.cpp
LOCAL_SRC_FILES += ting/timer.cpp
LOCAL_SRC_FILES += ting/WaitSet.cpp

#LOCAL_CFLAGS := -DDEBUG

LOCAL_LDLIBS    := -llog

include $(BUILD_SHARED_LIBRARY)
