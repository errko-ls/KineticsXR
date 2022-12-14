/*
 *
 * Copyright (c) 2022, KineticsXR
 * All rights reserved.
 *
 * BSD License
 *
 * Redistribution and use in source and binary forms, with or without modification, are
 * permitted provided that the following conditions are met:
 * - Redistributions of source code must retain the above copyright notice, this list of
 *  conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice, this list
 *  of conditions and the following disclaimer in the documentation and/or other materia
 * ls provided with the distribution.
 * - Neither the name of the "KineticsXR" nor the names of its contributors may be u
 * sed to endorse or promote products derived from this software without specific prior
 * written permission.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY E
 * XPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES O
 * F MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SH
 * ALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENT
 * AL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROC
 * UREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS I
 * NTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRI
 * CT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF T
 * HE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define LOG_TAG "KXControllerNative"
#include <vendor/kineticsxr/hardware/nordic/1.0/INordic.h>
#include <hidl/Status.h>
#include <hidl/LegacySupport.h>
#include <utils/misc.h>
#include <hidl/HidlSupport.h>
#include <stdio.h>
#include "jni.h"
#include <android/log.h>
using ::android::hardware::hidl_string;
using ::android::sp;
using vendor::kineticsxr::hardware::nordic::V1_0::INordic;
using android::hardware::hidl_string;
using android::hardware::hidl_handle;
using ::android::hidl::memory::V1_0::IMemory;
static const char *classPathName = "com/kineticsxr/service/nordic/bridge/HalNordicJni";
jint native_create(JNIEnv * /*env*/, jobject /*thiz*/) {
    android::sp <INordic> service = INordic::getService();
    if (service == nullptr) {
        printf("Failed to get service\n");
        ALOGE("native_reate Failed to get service!!!");
        return -1;
    }
    int res = service->Nordic_Create();
    return res;
}
jint native_destroy(JNIEnv * /*env*/, jobject /*thiz*/) {
    android::sp <INordic> service = INordic::getService();
    if (service == nullptr) {
        printf("Failed to get service\n");
        ALOGE("native_destroy Failed to get service!!!");
        return -1;
    }
    int res = service->Nordic_Destroy();
    return res;
}
jint native_start(JNIEnv * /*env*/, jobject /*thiz*/) {
    android::sp <INordic> service = INordic::getService();
    if (service == nullptr) {
        printf("Failed to get service\n");
        ALOGE("native_start Failed to get service!!!");
        return -1;
    }
    int res = service->Nordic_Start();

    return res;
}
jint native_stop(JNIEnv * /*env*/, jobject /*thiz*/) {
    android::sp <INordic> service = INordic::getService();
    if (service == nullptr) {
        printf("Failed to get service\n");
        ALOGE("native_stop Failed to get service!!!");
        return -1;
    }
    int res = service->Nordic_Stop();
    return res;
}
jfloat native_get_nordic_version(JNIEnv * /*env*/, jobject /*thiz*/) {
    android::sp <INordic> service = INordic::getService();
    if (service == nullptr) {
        printf("Failed to get service\n");
        ALOGE("native_get_nordic_version Failed to get service!!!");
        return -1;
    }
    float res = service->Nordic_Get_Nordic_Version();
    return res;
}
jfloat native_get_controller_version(JNIEnv * /*env*/, jobject /*thiz*/, jint type) {
    android::sp <INordic> service = INordic::getService();
    if (service == nullptr) {
        printf("Failed to get service\n");
        ALOGE("native_get_controller_version Failed to get service!!!");
        return -1;
    }
    float res = service->Nordic_Get_Controller_Version(type == 2 ? 0 : 1);
    return res;
}
jint native_bind_controller(JNIEnv * /*env*/, jobject /*thiz*/, jint type) {
    android::sp <INordic> service = INordic::getService();
    if (service == nullptr) {
        printf("Failed to get service\n");
        ALOGE("native_bind_controller Failed to get service!!!");
        return -1;
    }
    int res = service->Nordic_Bind_Controller(type == 2 ? 0 : 1);
    return res;
}
jint native_unbind_controller(JNIEnv * /*env*/, jobject /*thiz*/, jint type) {
    android::sp <INordic> service = INordic::getService();
    if (service == nullptr) {
        printf("Failed to get service\n");
        ALOGE("native_unbind_controller Failed to get service!!!");
        return -1;
    }
    int res = service->Nordic_Unbind_Controller(type == 2 ? 0 : 1);
    return res;
}
jint native_cancel_bind(JNIEnv * /*env*/, jobject /*thiz*/) {
    android::sp <INordic> service = INordic::getService();
    if (service == nullptr) {
        printf("Failed to get service\n");
        ALOGE("native_cancel_bind Failed to get service!!!");
        return -1;
    }
    int res = service->Nordic_Cancel_Bind();
    return res;
}
jint native_get_bind_state(JNIEnv * /*env*/, jobject /*thiz*/) {
    android::sp <INordic> service = INordic::getService();
    if (service == nullptr) {
        printf("Failed to get service\n");
        ALOGE("native_get_bind_state Failed to get service!!!");
        return -1;
    }
    int res = service->Nordic_Get_Bind_State();
    return res;
}
void native_getMem(JNIEnv *env, jobject /*thiz*/, jintArray data) {
    android::sp <INordic> service = INordic::getService();
    int m_size = 0;
    int fd = -1;
    if (service == nullptr) {
        printf("Failed to get service\n");
        ALOGE("native_getMem Failed to get service!!!");
        fd = -1;
        m_size = 0;
    } else {
        service->Nordic_Get_Memory([&](hidl_handle _handle, int32_t _size) {
            fd = dup(_handle->data[0]);
            m_size = _size;
        });
    }
    jint *arr = env->GetIntArrayElements(data, 0);
    arr[0] = m_size;
    arr[1] = fd;
    env->ReleaseIntArrayElements(data, arr, 0);
}
jint native_setVibration(JNIEnv * /*env*/, jobject /*thiz*/, jlong duration_ns, jfloat frequency, jfloat amplitude, jint type) {
    android::sp <INordic> service = INordic::getService();
    if (service == nullptr) {
        printf("Failed to get service\n");
        ALOGE("native_setVibration Failed to get service!!!");
        return -1;
    }
    float durationS = duration_ns * 1e-9 / 3.1f;
    int times = (int) durationS;
    durationS = duration_ns * 1e-9 - times * 3.1f;
    uint32_t bit16_31 = 0x82000000;
    uint32_t bit12_15 = (uint16_t) amplitude * 15 << 12;
    uint32_t bit8_11 = (uint16_t) frequency * 15 << 8;
    uint32_t bit3_7_max = 0xF8;
    uint32_t bit3_7 = (uint16_t)(durationS * 10) << 3;
    uint32_t bit2 = 0x1 << 2;
    uint32_t bit0_1 = type == 2 ? 0x00 : 0x01;
    uint32_t value = bit16_31 | bit12_15 | bit8_11 | bit3_7_max | bit2 | bit0_1;
    for (int i = 0; i < times; i++) {
        if (service->Nordic_Set_Vibration(value) < 0) {
            return -1;
        }
    }
    value = bit16_31 | bit12_15 | bit8_11 | bit3_7 | bit2 | bit0_1;
    return service->Nordic_Set_Vibration(value);
}
static JNINativeMethod ledMethod[] = {
        {"nativeCreate",                "()I",     (void *) native_create},
        {"nativeDestroy",                "()I",     (void *) native_destroy},
        {"nativeStart",                "()I",     (void *) native_start},
        {"nativeStop",                 "()I",     (void *) native_stop},
        {"nativeGetMem",               "([I)V",   (void *) native_getMem},
        {"nativeSetVibration",         "(JFFI)I", (void *) native_setVibration},
        {"nativeGetNordicVersion",     "()F",     (void *) native_get_nordic_version},
        {"nativeGetControllerVersion", "(I)F",    (void *) native_get_controller_version},
        {"nativeBindController",       "(I)I",    (void *) native_bind_controller},
        {"nativeUnbindController",     "(I)I",    (void *) native_unbind_controller},
        {"nativeCancelBindController", "()I",     (void *) native_cancel_bind},
        {"nativeGetBindControllerState",    "()I",     (void *) native_get_bind_state},
};
jint JNI_OnLoad(JavaVM *vm, void * /*reserved*/) {
    jint ret;
    JNIEnv *env = NULL;
    ret = vm->GetEnv((void **) &env, JNI_VERSION_1_4);
    if (ret != 0) {
        ALOGE("GetEnv Failed ret != 0");
        return -1;
    }
    jclass cls = env->FindClass(classPathName);
    if (cls == NULL) {
        ALOGE("FindClass Failed cls == NULL");
        return JNI_FALSE;
    }
    ret = env->RegisterNatives(cls, ledMethod, sizeof(ledMethod) / sizeof(ledMethod[0]));
    if (ret < 0) {
        ALOGE("RegisterNatives Failed ret < 0");
        return JNI_FALSE;
    }
    return JNI_VERSION_1_4;
}