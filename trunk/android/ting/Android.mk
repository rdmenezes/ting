LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

#TARGET_BUILD_TYPE := debug

#LOCAL_ARM_MODE := arm #arm or thumb

LOCAL_MODULE := ting

SRC_BASE_DIR := 

LOCAL_SRC_FILES :=
LOCAL_SRC_FILES += $(SRC_BASE_DIR)ting/fs/File.cpp
LOCAL_SRC_FILES += $(SRC_BASE_DIR)ting/fs/FSFile.cpp
LOCAL_SRC_FILES += $(SRC_BASE_DIR)ting/mt/MsgThread.cpp
LOCAL_SRC_FILES += $(SRC_BASE_DIR)ting/mt/Mutex.cpp
LOCAL_SRC_FILES += $(SRC_BASE_DIR)ting/mt/Queue.cpp
LOCAL_SRC_FILES += $(SRC_BASE_DIR)ting/mt/Semaphore.cpp
LOCAL_SRC_FILES += $(SRC_BASE_DIR)ting/mt/Thread.cpp
LOCAL_SRC_FILES += $(SRC_BASE_DIR)ting/net/HostNameResolver.cpp
LOCAL_SRC_FILES += $(SRC_BASE_DIR)ting/net/IPAddress.cpp
LOCAL_SRC_FILES += $(SRC_BASE_DIR)ting/net/Lib.cpp
LOCAL_SRC_FILES += $(SRC_BASE_DIR)ting/net/Socket.cpp
LOCAL_SRC_FILES += $(SRC_BASE_DIR)ting/net/TCPServerSocket.cpp
LOCAL_SRC_FILES += $(SRC_BASE_DIR)ting/net/TCPSocket.cpp
LOCAL_SRC_FILES += $(SRC_BASE_DIR)ting/net/UDPSocket.cpp
LOCAL_SRC_FILES += $(SRC_BASE_DIR)ting/Ref.cpp
LOCAL_SRC_FILES += $(SRC_BASE_DIR)ting/timer.cpp
LOCAL_SRC_FILES += $(SRC_BASE_DIR)ting/WaitSet.cpp

#LOCAL_CFLAGS := -DDEBUG

LOCAL_LDLIBS    := -llog

#include $(BUILD_SHARED_LIBRARY)
include $(BUILD_STATIC_LIBRARY)
