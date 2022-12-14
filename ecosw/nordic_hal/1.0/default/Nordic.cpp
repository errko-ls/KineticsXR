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

#include "Nordic.h"
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "KXControllerHal"
#undef ALOGW
#undef ALOGE
#undef ALOGD
#include <android/log.h>
#include <hidlmemory/mapping.h>
#include <android/hidl/memory/1.0/IMemory.h>
#include "qvr_helper.h"
#include "KXControllerHelper.h"
#define  ALOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#define  ALOGD(...)  //__android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define  ALOGW(...)  // __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
using ::android::hidl::memory::V1_0::IMemory;
using ::android::hidl::allocator::V1_0::IAllocator;
using ::android::hidl::base::V1_0::IBase;
namespace vendor::kineticsxr::hardware::nordic::implementation {
char *ctrl_config_path = "/vendor/etc/qvr/ivctrl.yaml";
char *slam_config_path = "/vendor/etc/qvr/device_calibration.xml";
char *output_path = "/vendor/etc/qvr/";
kx_controller_helper_t *mControllerHelper = nullptr;
void camera_long_callback(uint64_t ts, char *camdata1, char *camdata2, int w, int h) {
    ALOGW("camera_long_callback w:%d, h:%d", w, h);
    if (mControllerHelper) {
        SC::qvr_need_camera(KX_Controller_Helper_Is_Connected(mControllerHelper));
        KX_Controller_Helper_Cam_Long_Callback(mControllerHelper, ts, camdata1, camdata2, w, h);
    }
}
void camera_short_callback(uint64_t ts, char *camdata1, char *camdata2, float *twb,
                       float *linear_velocity, float *angular_velocity,
                       uint64_t long_ts, float *l_twb, float *long_linear_velocity,
                       float *long_angular_velocity,
                       int w, int h) {
    ALOGW("camera_short_callback w:%d, h:%d", w, h);
    if (mControllerHelper) {
        SC::qvr_need_camera(KX_Controller_Helper_Is_Connected(mControllerHelper));
        KX_Controller_Helper_Cam_Short_Callback(mControllerHelper, ts, camdata1, camdata2, twb,
                                               linear_velocity, angular_velocity,
                                               long_ts, l_twb, long_linear_velocity,
                                               long_angular_velocity,
                                               w, h);
    }
}
void pause_algo_callback() {
    ALOGD("sa_status_nor pause_algo_callback");
    if (mControllerHelper) {
        KX_Controller_Helper_Pause(mControllerHelper);
    }
}
void resume_algo_callback() {
    ALOGD("sa_status_nor resume_algo_callback");
    if (mControllerHelper) {
        KX_Controller_Helper_Resume(mControllerHelper);
    }
}
Nordic::Nordic() {
    ALOGD("%s:: start finish!!", __FUNCTION__);
    if(!mControllerHelper){
        mControllerHelper = KX_Controller_Helper_Init();
        if (!mControllerHelper) {
            ALOGE("%s:: init mControllerHelper fail!!", __FUNCTION__);
        }
	}
	if(mControllerHelper){
		mSize = KX_Controller_Helper_Buffer_Size(mControllerHelper);
	}
    int32_t pageSize = getpagesize();
    sp <IAllocator> allocator = IAllocator::getService("ashmem");
    int32_t len = ((mSize + pageSize - 1) & ~(pageSize - 1));
    allocator->allocate(len, [&](bool success, const hidl_memory &mem) {
        if (!success) {
            ALOGE("memory allocate failed!!!");
            return;
        }
        m_hidl_handle = native_handle_clone(mem.handle());
        m_hidl_heap = hidl_memory("ashmem", m_hidl_handle, len);
        m_hidl_heapMemory = mapMemory(m_hidl_heap);
        if (m_hidl_heapMemory == nullptr) {
            ALOGE("memory map failed!!!");
            native_handle_close(m_hidl_handle); // close FD for the shared memory
            native_handle_delete(m_hidl_handle);
            m_hidl_heap = hidl_memory();
            m_hidl_handle = nullptr;
            return;
        }
        m_hidl_heapMemData = m_hidl_heapMemory->getPointer();
        if (m_hidl_heapMemData != NULL) {
			//do nothing
        } else {
            ALOGE("get memory data failed!!!");
            native_handle_close(m_hidl_handle); // close FD for the shared memory
            native_handle_delete(m_hidl_handle);
            m_hidl_heap = hidl_memory();
            m_hidl_handle = nullptr;
            return;
        }
    });
    if (mControllerHelper) {
        KX_Controller_Helper_Create(mControllerHelper, m_hidl_heapMemData, ctrl_config_path, slam_config_path, output_path);
    }
}
Nordic::~Nordic() {
    if (m_hidl_heapMemory != nullptr) {
        m_hidl_heapMemData = nullptr;
        m_hidl_heapMemory.clear(); // The destructor will trigger munmap
    }
    if (m_hidl_handle) {
        native_handle_close(m_hidl_handle); // close FD for the shared memory
        native_handle_delete(m_hidl_handle);
    }
}
// Methods from ::vendor::kineticsxr::hardware::nordic::V1_0::INordic follow.
Return<void> Nordic::helloWorld(const hidl_string &name, helloWorld_cb _hidl_cb) {
    char buf[128];
    ::memset(buf, 0, 128);
    ::snprintf(buf, 128, "Starting %s", name.c_str());
    hidl_string result(buf);
    _hidl_cb(result);
    return Void();
}
Return <int32_t> Nordic::Nordic_Create() {
    ALOGD("Nordic_Create m_hidl_heapMemData:%d",m_hidl_heapMemData);
	SC::qvr_setLongCallback(camera_long_callback);
	SC::qvr_setShortCallback(camera_short_callback);
	SC::qvr_create();
	//SC::qvr_need_camera(true);
	SC::qvr_setPauseCallback(pause_algo_callback);
	SC::qvr_setResumeCallback(resume_algo_callback);
    if (mControllerHelper) {
        KX_Controller_Helper_Start(mControllerHelper);
    }
    return 0;
}
Return <int32_t> Nordic::Nordic_Destroy() {
    ALOGD("Nordic_Destroy");
	SC::qvr_destory();
    if (mControllerHelper) {
        KX_Controller_Helper_Stop(mControllerHelper);
        KX_Controller_Helper_Destory(mControllerHelper);
    }
    return 0;
}
Return <int32_t> Nordic::Nordic_Start() {
    ALOGD("sa_status_nor Nordic_Start");
	SC::qvr_resume();
    if (mControllerHelper) {
        KX_Controller_Helper_Resume(mControllerHelper);
    }
    return 0;
}
Return<void> Nordic::Nordic_Get_Memory(Nordic_Get_Memory_cb _hidl_cb) {
    ALOGD("Nordic_Get_Memory");
    if (m_hidl_handle) {
        ALOGD("m_hidl_handle data[0] = %d", m_hidl_handle->data[0]);
    } else {
        ALOGE("m_hidl_handle is NULL!");
    }
    _hidl_cb(m_hidl_handle, mSize);
    return Void();
}
Return <int32_t> Nordic::Nordic_Stop() {
    ALOGD("sa_status_nor Nordic_Stop");
    int32_t ret = 0;
	SC::qvr_pasue();
    if (mControllerHelper) {
        KX_Controller_Helper_Pause(mControllerHelper);
    }
    return ret;
}
Return<float> Nordic::Nordic_Get_Nordic_Version() {
    float ret = -1.0f;
    if (mControllerHelper) {
        ret = KX_Controller_Helper_Get_Nordic_Version(mControllerHelper);
    }
	ALOGD("Nordic_Get_Nordic_Version:%f",ret);
    return ret;
}
Return<float> Nordic::Nordic_Get_Controller_Version(int32_t lr) {
    float ret = -1.0f;
    if (mControllerHelper) {
        ret = KX_Controller_Helper_Get_Controller_Version(mControllerHelper, lr);
    }
	ALOGD("Nordic_Get_Controller_Version:%f",ret);
    return ret;
}
Return <int32_t> Nordic::Nordic_Bind_Controller(int32_t lr) {
    int32_t ret = 0;
    if (mControllerHelper) {
        ret = KX_Controller_Helper_Bind_Controller(mControllerHelper, lr);
    }
	ALOGD("KX_Controller_Helper_Bind_Controller:%d",ret);
    return ret;
}
Return <int32_t> Nordic::Nordic_Unbind_Controller(int32_t lr) {
    int32_t ret = 0;
    if (mControllerHelper) {
        ret = KX_Controller_Helper_Unbind_Controller(mControllerHelper, lr);
    }
	ALOGD("KX_Controller_Helper_Unbind_Controller:%d",ret);
    return ret;
}
Return <int32_t> Nordic::Nordic_Cancel_Bind() {
    int32_t ret = 0;
    if (mControllerHelper) {
        ret = KX_Controller_Helper_Cancel_Bind(mControllerHelper);
    }
	ALOGD("KX_Controller_Helper_Cancel_Bind:%d",ret);
    return ret;
}
Return <int32_t> Nordic::Nordic_Get_Bind_State() {
    int32_t ret = -1;
    if (mControllerHelper) {
        ret = KX_Controller_Helper_Get_Bind_State(mControllerHelper);
    }
	ALOGD("KX_Controller_Helper_Get_Bind_State:%d",ret);
    return ret;
}
Return <int32_t> Nordic::Nordic_Set_Vibration(int32_t value) {
    int32_t ret = -1;
    if (mControllerHelper) {
        ret = KX_Controller_Helper_Set_Vibration(mControllerHelper, value);
    }
	ALOGD("KX_Controller_Helper_Set_Vibration:%d",ret);
    return ret;
}
Return <int32_t> Nordic::Nordic_Enter_Dfu() {
    int32_t ret = 0;
	ALOGD("Nordic_EnterDfu");
    if (mControllerHelper) {
        KX_Controller_Helper_Enter_Dfu(mControllerHelper);
    }
    return ret;
}
// Methods from ::android::hidl::base::V1_0::IBase follow.
V1_0::INordic *HIDL_FETCH_INordic(const char * /* name */) {
    return new Nordic();
}
//
}  // namespace vendor::kineticsxr::hardware::nordic::implementation
