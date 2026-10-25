LOCAL_PATH := $(call my-dir)

# --- Define CURL Prebuilt ---
include $(CLEAR_VARS)
LOCAL_MODULE := curl_static
LOCAL_SRC_FILES := src/static/curl/$(TARGET_ARCH_ABI)/libcurl.a
include $(PREBUILT_STATIC_LIBRARY)

# --- Module: cnative (ESP) ---
include $(CLEAR_VARS)
LOCAL_MODULE    := cnative
LOCAL_SRC_FILES := \
    src/Client.cpp \
    src/Namegun.cpp \
    src/Socket/client.cpp \
    src/MemfdHelper.cpp

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/src \
    $(LOCAL_PATH)/src/Includes \
    $(LOCAL_PATH)/src/Includes/curl \
    $(LOCAL_PATH)/src/Socket \
    $(LOCAL_PATH)/src/Unity

LOCAL_CFLAGS    := -Wno-error=format-security -fpermissive -fvisibility=hidden -fexceptions -frtti -DNDEBUG -Os -fdata-sections -ffunction-sections
LOCAL_CPPFLAGS  := -fvisibility=hidden -DNDEBUG -std=c++17 -fexceptions -frtti -Os -fdata-sections -ffunction-sections
LOCAL_LDFLAGS   := -Wl,--exclude-libs,ALL -s -latomic -Wl,--gc-sections -Wl,--icf=safe
LOCAL_LDLIBS    := -llog -landroid -lz
LOCAL_STATIC_LIBRARIES := curl_static
LOCAL_ARM_MODE  := arm
include $(BUILD_SHARED_LIBRARY)

# --- Module: BKLKILL (Aimbot Payload) ---
include $(CLEAR_VARS)
LOCAL_MODULE    := BKLKILL
# Add your Dobby cpp file here!
LOCAL_SRC_FILES := src/Server.cpp src/Socket/server.cpp src/Includes/Dobby/dobby.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/src $(LOCAL_PATH)/src/Includes
LOCAL_CPPFLAGS  := -fvisibility=hidden -DNDEBUG -std=c++17 -fexceptions -frtti
LOCAL_CFLAGS    := -Wno-error=format-security -fpermissive -fvisibility=hidden -fexceptions -frtti -DNDEBUG
LOCAL_LDFLAGS   := -Wl,--exclude-libs,ALL -s -latomic
LOCAL_LDLIBS    := -llog -lz
LOCAL_ARM_MODE  := arm
# CHANGED THIS LINE:
include $(BUILD_SHARED_LIBRARY) 
