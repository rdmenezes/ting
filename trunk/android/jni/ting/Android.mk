LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

#TARGET_BUILD_TYPE := debug

#LOCAL_ARM_MODE   := arm #arm or thumb

LOCAL_MODULE    := ting

LOCAL_SRC_FILES :=
LOCAL_SRC_FILES += ting/File.cpp
LOCAL_SRC_FILES += ting/FSFile.cpp
LOCAL_SRC_FILES += ting/Ref.cpp
LOCAL_SRC_FILES += ting/Socket.cpp
LOCAL_SRC_FILES += ting/Thread.cpp
LOCAL_SRC_FILES += ting/Timer.cpp
LOCAL_SRC_FILES += ting/WaitSet.cpp

#LOCAL_CFLAGS := -DDEBUG

LOCAL_LDLIBS    := -llog

include $(BUILD_SHARED_LIBRARY)
