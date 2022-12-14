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

#ifndef KX_CONTROLLERHELPER_H
#define KX_CONTROLLERHELPER_H
#include <jni.h>
#include <stdint.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <math.h>
#include <cutils/log.h>
#include <functional>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct kx_controller_helper_ops {
    void (*controller_helper_create)(void *heap_mem_data, char *ctrl_config_path, char *slam_config_path, char *output_path);
    void (*controller_helper_start)();
    void (*controller_helper_stop)();
    void (*controller_helper_destory)();
    bool (*controller_helper_performhapticfeedback)(long durationNs, float frequency, float amplitude, int type);
    void (*controller_helper_resume)();
    void (*controller_helper_pause)();
    void (*controller_helper_cam_callback)(uint64_t ts,char *camdata,float * twb,char * l_camdata,float * l_twb,int w,int h);
    bool (*controller_helper_is_connected)();
    int32_t (*controller_helper_buffer_size)();
    void (*controller_helper_cam_short_callback)(uint64_t ts,char *camdata1,char *camdata2,float * twb,
                                                  float *linear_velocity,float *angular_velocity,
                                                  uint64_t long_ts,float * l_twb,float *long_linear_velocity,
                                                  float *long_angular_velocity,
                                                  int w,int h);
												  
    void (*controller_helper_cam_long_callback)(uint64_t ts,char *camdata1,char *camdata2,int w,int h);
    float (*controller_helper_get_nordic_version)();
    float (*controller_helper_get_controller_version)(int32_t lr);
    int32_t (*controller_helper_bind_controller)(int32_t lr);
    int32_t (*controller_helper_unbind_controller)(int32_t lr);
    int32_t (*controller_helper_cancel_bind)();
    int32_t (*controller_helper_get_bind_state)();
    int32_t (*controller_helper_set_vibration)(int32_t value);
    int32_t (*controller_helper_enter_dfu)();
	
} kx_controller_helper_ops_t;
#define KX_CONTROLLER_HELPER_LIB "libkxcontroller.so"
typedef struct {
    void *libHandle;
    kx_controller_helper_ops_t *ops;
    int api_version;
} kx_controller_helper_t;
static inline kx_controller_helper_t *KX_Controller_Helper_Init() {
    kx_controller_helper_t *me = (kx_controller_helper_t *)malloc(sizeof(kx_controller_helper_t));
    if (!me) return NULL;
    me->libHandle = dlopen(KX_CONTROLLER_HELPER_LIB, RTLD_NOW);
    if (!me->libHandle) {
        __android_log_print(ANDROID_LOG_INFO, "KX_CONTROLLER_HELPER_LIB", "KX_CONTROLLER_HELPER_LIB %s", dlerror());
        free(me);
        return NULL;
    }
    me->ops = (kx_controller_helper_ops_t *) malloc(sizeof(kx_controller_helper_ops_t));
    if (!me->ops) {
        dlclose(me->libHandle);
        free(me);
        return NULL;
    }
    me->api_version = 1;
    typedef void (*controller_helper_create_fn)(void *heap_mem_data, char *ctrl_config_path, char *slam_config_path, char *output_path);
    me->ops->controller_helper_create = (controller_helper_create_fn) dlsym(me->libHandle, "Controller_Create");
    typedef void (*controller_helper_start_fn)();
    me->ops->controller_helper_start = (controller_helper_start_fn) dlsym(me->libHandle, "Controller_Start");
    typedef void (*controller_helper_stop_fn)();
    me->ops->controller_helper_stop = (controller_helper_stop_fn) dlsym(me->libHandle, "Controller_Stop");
    typedef void (*controller_helper_destory_fn)();
    me->ops->controller_helper_destory = (controller_helper_destory_fn) dlsym(me->libHandle, "Controller_Destory");
    typedef bool (*controller_helper_performhapticfeedback_fn)(long durationNs, float frequency, float amplitude, int type);
    me->ops->controller_helper_performhapticfeedback = (controller_helper_performhapticfeedback_fn) dlsym(me->libHandle, "Controller_PerformHapticFeedback");
    typedef void (*controller_helper_resume_fn)();
    me->ops->controller_helper_resume = (controller_helper_resume_fn) dlsym(me->libHandle, "Controller_Resume");
    typedef void (*controller_helper_pause_fn)();
    me->ops->controller_helper_pause = (controller_helper_pause_fn) dlsym(me->libHandle, "Controller_Pause");
    typedef void (*controller_helper_cam_callback_fn)(uint64_t ts,char *camdata,float * twb,char * l_camdata,float * l_twb,int w,int h);
    me->ops->controller_helper_cam_callback = (controller_helper_cam_callback_fn) dlsym(me->libHandle, "Controller_Cam_Callback");
	typedef void (*controller_helper_cam_short_callback_fn)(uint64_t ts,char *camdata1,char *camdata2,float * twb,
                                                  float *linear_velocity,float *angular_velocity,
                                                  uint64_t long_ts,float * l_twb,float *long_linear_velocity,
                                                  float *long_angular_velocity,
                                                  int w,int h);
    me->ops->controller_helper_cam_short_callback = (controller_helper_cam_short_callback_fn) dlsym(me->libHandle, "Controller_Cam_Short_Callback");
    typedef void (*controller_helper_cam_long_callback_fn)(uint64_t ts,char *camdata1,char *camdata2,int w,int h);
    me->ops->controller_helper_cam_long_callback = (controller_helper_cam_long_callback_fn) dlsym(me->libHandle, "Controller_Cam_Long_Callback");
    typedef bool (*controller_helper_is_connected_fn)();
    me->ops->controller_helper_is_connected = (controller_helper_is_connected_fn) dlsym(me->libHandle, "Controller_Is_Connected");
    typedef int32_t (*controller_helper_buffer_size_fn)();
    me->ops->controller_helper_buffer_size = (controller_helper_buffer_size_fn) dlsym(me->libHandle, "Controller_Buffer_Size");
    typedef float (*controller_helper_get_nordic_version_fn)();
    me->ops->controller_helper_get_nordic_version = (controller_helper_get_nordic_version_fn) dlsym(me->libHandle, "Controller_Get_Nordic_Version");
    typedef float (*controller_helper_get_controller_version_fn)(int32_t lr);
    me->ops->controller_helper_get_controller_version = (controller_helper_get_controller_version_fn) dlsym(me->libHandle, "Controller_Get_Controller_Version");
    typedef int32_t (*controller_helper_bind_controller_fn)(int32_t lr);
    me->ops->controller_helper_bind_controller = (controller_helper_bind_controller_fn) dlsym(me->libHandle, "Controller_Bind_Controller");
    typedef int32_t (*controller_helper_unbind_controller_fn)(int32_t lr);
    me->ops->controller_helper_unbind_controller = (controller_helper_unbind_controller_fn) dlsym(me->libHandle, "Controller_Unbind_Controller");
    typedef int32_t (*controller_helper_cancel_bind_fn)();
    me->ops->controller_helper_cancel_bind = (controller_helper_cancel_bind_fn) dlsym(me->libHandle, "Controller_Cancel_Bind");
    typedef int32_t (*controller_helper_get_bind_state_fn)();
    me->ops->controller_helper_get_bind_state = (controller_helper_get_bind_state_fn) dlsym(me->libHandle, "Controller_Get_Bind_State");
    typedef int32_t (*controller_helper_set_vibration_fn)(int32_t value);
    me->ops->controller_helper_set_vibration = (controller_helper_set_vibration_fn) dlsym(me->libHandle, "Controller_Set_Vibration");
	typedef int32_t (*controller_helper_enter_dfu_fn)();
    me->ops->controller_helper_enter_dfu = (controller_helper_enter_dfu_fn) dlsym(me->libHandle, "Controller_Enter_Dfu");
    return me;
}
static inline void KX_Controller_Helper_Die(kx_controller_helper_t *me) {
    if (!me) return;
    if (me->libHandle) {
        dlclose(me->libHandle);
    }
    free(me);
    me = NULL;
}
static inline int KX_Controller_Helper_Create(kx_controller_helper_t *me, void *heap_mem_data, char *ctrl_config_path, char *slam_config_path, char *output_path) {
    if (!me) return -2;
    if (me->ops->controller_helper_create) {
		me->ops->controller_helper_create(heap_mem_data, ctrl_config_path, slam_config_path, output_path);
        return 0;
    }
    return -1;
}
static inline int KX_Controller_Helper_Start(kx_controller_helper_t *me) {
    if (!me) return -2;
    if (me->ops->controller_helper_start) {
		me->ops->controller_helper_start();
        return 1;
    }
    return -1;
}
static inline int KX_Controller_Helper_Stop(kx_controller_helper_t *me) {
    if (!me) return -2;
    if (me->ops->controller_helper_stop) {
		me->ops->controller_helper_stop();
        return 1;
    }
    return -1;
}
static inline int KX_Controller_Helper_Destory(kx_controller_helper_t *me) {
    if (!me) return -2;
    if (me->ops->controller_helper_destory) {
        me->ops->controller_helper_destory();
        return 1;
    }
    return -1;
}
static inline int KX_Controller_Helper_PerformHapticFeedback(kx_controller_helper_t *me, long durationNs, float frequency, float amplitude, int type) {
    if (!me) return -2;
    if (me->ops->controller_helper_performhapticfeedback) {
        return me->ops->controller_helper_performhapticfeedback(durationNs, frequency, amplitude, type)?1:0;
    }
    return -1;
}
static inline int KX_Controller_Helper_Resume(kx_controller_helper_t *me) {
    if (!me) return -2;
    if (me->ops->controller_helper_resume) {
        me->ops->controller_helper_resume();
        return 1;
    }
    return -1;
}
static inline int KX_Controller_Helper_Pause(kx_controller_helper_t *me) {
    if (!me) return -2;
    if (me->ops->controller_helper_pause) {
        me->ops->controller_helper_pause();
        return 1;
    }
    return -1;
}
static inline int KX_Controller_Helper_Cam_Short_Callback(kx_controller_helper_t *me, uint64_t ts,char *camdata1,char *camdata2,float * twb,
                                                  float *linear_velocity,float *angular_velocity,
                                                  uint64_t long_ts,float * l_twb,float *long_linear_velocity,
                                                  float *long_angular_velocity,
                                                  int w,int h) {
    if (!me) return -2;
    if (me->ops->controller_helper_cam_short_callback) {
        me->ops->controller_helper_cam_short_callback(ts, camdata1, camdata2, twb,
                                                   linear_velocity, angular_velocity,
                                                   long_ts, l_twb, long_linear_velocity,
                                                   long_angular_velocity,
                                                   w, h);
        return 1;
    }
    return -1;
}
static inline int KX_Controller_Helper_Cam_Long_Callback(kx_controller_helper_t *me, uint64_t ts,char *camdata1,char *camdata2,int w,int h) {
    if (!me) return -2;
    if (me->ops->controller_helper_cam_long_callback) {
        me->ops->controller_helper_cam_long_callback(ts, camdata1, camdata2, w, h);
        return 1;
    }
    return -1;
}
static inline bool KX_Controller_Helper_Is_Connected(kx_controller_helper_t *me) {
    if (!me) return false;
    if (me->ops->controller_helper_is_connected) {
        return me->ops->controller_helper_is_connected();
    }
    return false;
}
static inline int32_t KX_Controller_Helper_Buffer_Size(kx_controller_helper_t *me) {
    if (!me) return -2;
    if (me->ops->controller_helper_buffer_size) {
        
        return me->ops->controller_helper_buffer_size();
    }
    return -1;
}
static inline float KX_Controller_Helper_Get_Nordic_Version(kx_controller_helper_t *me) {
    if (!me) return -2.0f;
    if (me->ops->controller_helper_get_nordic_version) {
        
        return me->ops->controller_helper_get_nordic_version();
    }
    return -1.0f;
}
static inline float KX_Controller_Helper_Get_Controller_Version(kx_controller_helper_t *me, int32_t lr) {
    if (!me) return -2.0f;
    if (me->ops->controller_helper_get_controller_version) {
        
        return me->ops->controller_helper_get_controller_version(lr);
    }
    return -1.0f;
}
static inline int32_t KX_Controller_Helper_Bind_Controller(kx_controller_helper_t *me, int32_t lr) {
    if (!me) return -2;
    if (me->ops->controller_helper_bind_controller) {
        
        return me->ops->controller_helper_bind_controller(lr);
    }
    return -1;
}
static inline int32_t KX_Controller_Helper_Unbind_Controller(kx_controller_helper_t *me, int32_t lr) {
    if (!me) return -2;
    if (me->ops->controller_helper_unbind_controller) {
        
        return me->ops->controller_helper_unbind_controller(lr);
    }
    return -1;
}
static inline int32_t KX_Controller_Helper_Cancel_Bind(kx_controller_helper_t *me) {
    if (!me) return -2;
    if (me->ops->controller_helper_cancel_bind) {
        
        return me->ops->controller_helper_cancel_bind();
    }
    return -1;
}
static inline int32_t KX_Controller_Helper_Get_Bind_State(kx_controller_helper_t *me) {
    if (!me) return -2;
    if (me->ops->controller_helper_get_bind_state) {
        
        return me->ops->controller_helper_get_bind_state();
    }
    return -1;
}
static inline int32_t KX_Controller_Helper_Set_Vibration(kx_controller_helper_t *me, int32_t value) {
    if (!me) return -2;
    if (me->ops->controller_helper_set_vibration) {
        
        return me->ops->controller_helper_set_vibration(value);
    }
    return -1;
}
static inline int32_t KX_Controller_Helper_Enter_Dfu(kx_controller_helper_t *me) {
    if (!me) return -2;
    if (me->ops->controller_helper_enter_dfu) {
        return me->ops->controller_helper_enter_dfu();
    }
    return -1;
}
#ifdef __cplusplus
}
#endif
#endif //KX_CONTROLLERHELPER_H
