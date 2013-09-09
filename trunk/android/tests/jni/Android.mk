LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

#TARGET_BUILD_TYPE := debug

#LOCAL_ARM_MODE   := arm #arm or thumb

LOCAL_MODULE    := ting_tests

LOCAL_SRC_FILES := main.cpp

LOCAL_CFLAGS := -DDEBUG

LOCAL_C_INCLUDES :=

LOCAL_LDLIBS    := -llog -landroid -lEGL -lGLESv1_CM

LOCAL_STATIC_LIBRARIES := android_native_app_glue ting

#LOCAL_SHARED_LIBRARIES := ting

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)
