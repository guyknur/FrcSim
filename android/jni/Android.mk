# Copyright (C) 2010 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#


SAMPLE_PATH := $(call my-dir)/../../src

# external-deps
LIBPNG_PATH := $(call my-dir)/../../../GamePlay/external-deps/png/lib/android/arm
ZLIB_PATH :=   $(call my-dir)/../../../GamePlay/external-deps/zlib/lib/android/arm
LUA_PATH :=    $(call my-dir)/../../../GamePlay/external-deps/lua/lib/android/arm
BULLET_PATH := $(call my-dir)/../../../GamePlay/external-deps/bullet/lib/android/arm
VORBIS_PATH := $(call my-dir)/../../../GamePlay/external-deps/oggvorbis/lib/android/arm
OPENAL_PATH := $(call my-dir)/../../../GamePlay/external-deps/openal/lib/android/arm

GHOUL_PATH := ../../Ghoul/android/obj/local/armeabi-v7a
JSONCPP_PATH := ../../JsonCPP/android/obj/local/armeabi-v7a

# gameplay
LOCAL_PATH := ../../GamePlay/gameplay/android/obj/local/armeabi
include $(CLEAR_VARS)
LOCAL_MODULE    := libgameplay
LOCAL_SRC_FILES := libgameplay.a
include $(PREBUILT_STATIC_LIBRARY)

# libpng
LOCAL_PATH := $(LIBPNG_PATH)
include $(CLEAR_VARS)
LOCAL_MODULE    := libpng 
LOCAL_SRC_FILES := libpng.a
include $(PREBUILT_STATIC_LIBRARY)

# libzlib
LOCAL_PATH := $(ZLIB_PATH)
include $(CLEAR_VARS)
LOCAL_MODULE    := libzlib
LOCAL_SRC_FILES := libzlib.a
include $(PREBUILT_STATIC_LIBRARY)

# liblua
LOCAL_PATH := $(LUA_PATH)
include $(CLEAR_VARS)
LOCAL_MODULE    := liblua
LOCAL_SRC_FILES := liblua.a
include $(PREBUILT_STATIC_LIBRARY)

# libbullet
LOCAL_PATH := $(BULLET_PATH)
include $(CLEAR_VARS)
LOCAL_MODULE    := libbullet
LOCAL_SRC_FILES := libbullet.a
include $(PREBUILT_STATIC_LIBRARY)

# libvorbis
LOCAL_PATH := $(VORBIS_PATH)
include $(CLEAR_VARS)
LOCAL_MODULE    := libvorbis
LOCAL_SRC_FILES := libvorbis.a
include $(PREBUILT_STATIC_LIBRARY)

# libOpenAL
LOCAL_PATH := $(OPENAL_PATH)
include $(CLEAR_VARS)
LOCAL_MODULE    := libOpenAL
LOCAL_SRC_FILES := libOpenAL.a
include $(PREBUILT_STATIC_LIBRARY)

# libGhoul
LOCAL_PATH := $(GHOUL_PATH)
include $(CLEAR_VARS)
LOCAL_MODULE    := libGhoul
LOCAL_SRC_FILES := libGhoul.a
include $(PREBUILT_STATIC_LIBRARY)

# libJsonCpp
LOCAL_PATH := $(JSONCPP_PATH)
include $(CLEAR_VARS)
LOCAL_MODULE    := libJsonCpp
LOCAL_SRC_FILES := libJsonCpp.a
include $(PREBUILT_STATIC_LIBRARY)

# FrcSim
LOCAL_PATH := $(SAMPLE_PATH)
NDK_TOOLCHAIN_VERSION=4.8
include $(CLEAR_VARS)

LOCAL_MODULE    := FrcSim
LOCAL_SRC_FILES := ../../GamePlay/gameplay/src/gameplay-main-android.cpp \
		Robot.cpp \
		FrcSim.cpp
LOCAL_CPP_FEATURES += rtti exceptions
LOCAL_LDLIBS    := -llog -landroid -lEGL -lGLESv2 -lOpenSLES 
LOCAL_CFLAGS    := -D__ANDROID__ -Wno-psabi 
LOCAL_CPPFLAGS += -g -std=gnu++11 -DDEBUG -DANDROID
LOCAL_C_INCLUDES := ../../GamePlay/external-deps/lua/include \
		../../GamePlay/external-deps/bullet/include \
		../../GamePlay/external-deps/png/include \
		../../GamePlay/external-deps/oggvorbis/include \
		../../GamePlay/external-deps/openal/include \
		../../GamePlay/gameplay/src \
		../../JsonCPP/include \
		../../Ghoul/include \
		../include

LOCAL_STATIC_LIBRARIES := android_native_app_glue libgameplay libpng libzlib liblua libbullet libvorbis libOpenAL JsonCpp Ghoul
include $(BUILD_SHARED_LIBRARY)
$(call import-module,android/native_app_glue)
