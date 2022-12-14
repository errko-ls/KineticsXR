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

#include "qvr_helper.h"
#include <string>
#include "QVRServiceClient.h"
#include "QVRCameraClient.h"
#include <thread>
#include <unistd.h>
#include <sys/system_properties.h>
#include "log_mgr.h"
#define USE_FRAME_BUFFER
#ifdef USE_FRAME_BUFFER
#include "image_buffer.h"
CameraFrame_t cam_frame_buffer[FRAME_BUFFER_SIZE];
int cam_frame_idx = 0;
#endif
namespace SC{
int startClient(bool withqvrClient);
int stopClient(bool withqvrClient);
qvrcamera_client_helper_t *mCameraClient;
qvrservice_client_helper_t *qvrHelper;
qvrcamera_device_helper_t *mCameraDevice = nullptr;
qvrcamera_device_helper_t *mSecCameraDevice = nullptr;
char mCameraName[64];
char mSecCameraName[64];
std::thread mThread;
bool clientStarted = false;
QVRSERVICE_VRMODE_STATE mcCurrentServiceState;
std::function<void()> mPauseCallback;
std::function<void()> mResumeCallback;
std::function<void(uint64_t ts,char *camdata1,char *camdata2,int w,int h)> mLongCallback;
std::function<void(uint64_t ts,char *camdata1,char *camdata2,float * twb,
                   float *linear_velocity,float *angular_velocity,
                   uint64_t long_ts,float * l_twb,float *long_linear_velocity,
                   float *long_angular_velocity,
                   int w,int h)> mShortCallback;
bool needCamera = false;
bool isScreenOff = false;
bool isResume = false;
bool start_qvr = false;
#ifdef SS
    bool use_2cam = true;
#else
    bool use_2cam = false;
#endif
void toMatrix(float quat[4], float tran[3], float out[16]) {
    float w = quat[3];
    float x = quat[0];
    float y = quat[1];
    float z = quat[2];
    float xx = x * x;
    float yy = y * y;
    float zz = z * z;
    float xy = x * y;
    float wz = w * z;
    float wy = w * y;
    float xz = x * z;
    float yz = y * z;
    float wx = w * x;
    out[0] = -1 * 2 * (xy + wz);
    out[4] = -1 * (1.0f - 2 * (xx + zz));
    out[8] = -1 * 2 * (yz - wx);
    out[12] = -1 * tran[1];
    out[1] = -1 * 2 * (xz - wy);
    out[5] = -1 * 2 * (yz + wx);
    out[9] = -1 * (1.0f - 2 * (xx + yy));
    out[13] = -1 * tran[2];
    out[2] = 1.0f - 2 * (yy + zz);
    out[6] = 2 * (xy - wz);
    out[10] = 2 * (wy + xz);
    out[14] = tran[0];
    out[3] = 0.0f;
    out[7] = 0.0f;
    out[11] = 0.0f;
    out[15] = 1.0f;
}
void qvr_status_callback(void *pCtx,QVRSERVICE_CLIENT_STATUS status, uint32_t arg1, uint32_t arg2){
    log_debug("sa_status qvr_status_callback %d %u %u",status,arg1,arg2);
    if(status == STATUS_STATE_CHANGED){
        if(arg1 == VRMODE_STARTED){
            if(isResume && !clientStarted){
                startClient(false);
            }
            if(isScreenOff == true){
                mResumeCallback();
                isScreenOff = false;
            }
        }
        if(arg1 == VRMODE_PAUSED || arg1 == VRMODE_HEADLESS || arg1 == VRMODE_STOPPED){
            if(clientStarted){
                stopClient(false);
            }
            if(arg1 == VRMODE_PAUSED && !isScreenOff){
                mPauseCallback();
                isScreenOff = true;
            }
        }
    }
}
void threadLoop() {
    log_debug("sa_status threadLoop");
    int32_t ret = -1;
    int32_t frameNumber = 0;
    int32_t lastFrameNumber = 0;
#ifndef USE_FRAME_BUFFER
    qvrcamera_frame_t cameraFrame;
    qvrcamera_frame_t cameraPeerFrame;
#endif
    int8_t isAutoExposure = 0;
    double lastLongTs = -1.0;
    std::string name = "FeedImageThread";
    pthread_setname_np(pthread_self(), name.c_str());
    int qRes;
    unsigned int retLen = 0;
    int64_t gQTimeToAndroidBoot = 0LL;
    qRes = QVRServiceClient_GetParam(qvrHelper, QVRSERVICE_TRACKER_ANDROID_OFFSET_NS, &retLen, NULL);
    if(qRes != QVR_SUCCESS){
        log_error("QVRServiceClient_GetParam(QVRSERVICE_TRACKER_ANDROID_OFFSET_NS - Length)", qRes);
    }else{
        char *pRetValue = new char[retLen + 1];
        if(pRetValue == NULL)
        {
            log_error("Unable to allocate %d bytes for return string!", retLen + 1);
        }else{
            memset(pRetValue, 0, retLen + 1);
            qRes = QVRServiceClient_GetParam(qvrHelper, QVRSERVICE_TRACKER_ANDROID_OFFSET_NS, &retLen, pRetValue);
            if(qRes != QVR_SUCCESS){
                log_error("QVRServiceClient_GetParam(QVRSERVICE_TRACKER_ANDROID_OFFSET_NS)", qRes);
            }else{
                gQTimeToAndroidBoot = strtoll(pRetValue, NULL, 0);
            }
            // No longer need the temporary buffer
            delete[] pRetValue;
        }
    }
    log_debug("sa_status %s:: before loop clientStarted:%d", __FUNCTION__,clientStarted);
    while (clientStarted) {
#ifdef USE_FRAME_BUFFER
        auto& cameraFrame = cam_frame_buffer[cam_frame_idx].cameraFrame;
        auto& cameraPeerFrame = cam_frame_buffer[cam_frame_idx].cameraPeerFrame;
        cam_frame_idx = (cam_frame_idx + 1) % FRAME_BUFFER_SIZE;
#endif
        if(!needCamera){
            if(mLongCallback){
                mLongCallback(0, nullptr, nullptr,0,0);
            }
            if(mShortCallback){
                mShortCallback(0,nullptr,nullptr, nullptr, nullptr, nullptr,0, nullptr, nullptr, nullptr,0,0);
            }
            usleep(5000);
            continue;
        }
        if (frameNumber == 0 || frameNumber == -1) {
            ret = QVRCameraDevice_GetCurrentFrameNumber(mCameraDevice, &frameNumber);
            if (ret < 0) {
                usleep(5000);
                log_error("%s:: QVRCameraDevice_GetCurrentFrameNumber failed ret=%d!", __FUNCTION__, ret);
                continue;
            }
        }
        log_info("%s:: frameNumber = %d ; lastFrameNumber = %d", __FUNCTION__, frameNumber, lastFrameNumber);
        if (frameNumber == lastFrameNumber) {
            frameNumber++;
        }
        memset(&cameraFrame, 0, sizeof(qvrcamera_frame_t));
        ret = QVRCameraDevice_GetFrame(mCameraDevice, &frameNumber, QVRCAMERA_MODE_BLOCKING, QVRCAMERA_MODE_NEWER_IF_AVAILABLE, &cameraFrame);
        if (ret < 0) {
            log_error("%s:: QVRCameraDevice_GetFrame with QVRCAMERA_MODE_BLOCKING frameNumber=%d failed, ret=%d",__FUNCTION__, frameNumber, ret);
            if (ret == QVR_CAM_EXPIRED_FRAMENUMBER || ret == QVR_CAM_DROPPED_FRAMENUMBER) {
                frameNumber++;
            }
            usleep(5000);
            continue;
        }
        ret = QVRCameraDevice_GetFrameParamI8(mCameraDevice, &cameraFrame, QVRCAMERA_FRAME_PARAM_I8_AUTO_EXPOSURE_ACTIVE, &isAutoExposure);
        if (ret < 0) {
            log_error("%s::%s QVRCAMERA_FRAME_PARAM_I8_AUTO_EXPOSURE_ACTIVE failed, ret=%d", __FUNCTION__, mCameraName, ret);
            continue;
        }
        int32_t peerFrameNumber = -1;
        if(!use_2cam){
            ret = QVRCameraDevice_GetFrameParamI32(mCameraDevice, &cameraFrame, QVRCAMERA_FRAME_PARAM_I32_PEER_FRAME_NUMBER, &peerFrameNumber);
            if (ret < 0 || peerFrameNumber < 0) {
                log_error("%s::%s QVRCAMERA_FRAME_PARAM_I32_PEER_FRAME_NUMBER failed, ret=%d", __FUNCTION__, mSecCameraName, ret);
                if (frameNumber > 0) {
                    QVRCameraDevice_ReleaseFrame(mCameraDevice, frameNumber);
                }
                usleep(5000);
                continue;
            }
        }
        if (true) {
            log_info("%s::[%s] GetFrame frameNumber=%d, cameraFrame.fn=%d, start_of_exposure_ts=%"
            PRIu64
            ", exposure=%d,"
            "len=%d, widthxheight=%dx%d, secondWxH=%dx%d, gain=%d, stride=%d, format=0x%x, isAutoExposure=%d, peerFrameNumber=%d\n",
                    __FUNCTION__, mCameraName, frameNumber, cameraFrame.fn, cameraFrame.start_of_exposure_ts,
                    cameraFrame.exposure, cameraFrame.len, cameraFrame.width, cameraFrame.height,
                    cameraFrame.secondary_width, cameraFrame.secondary_height, cameraFrame.gain,
                    cameraFrame.stride, cameraFrame.format, isAutoExposure, peerFrameNumber);
        }
        if (peerFrameNumber > 0) {
            ret = QVRCameraDevice_GetFrame(mSecCameraDevice, &peerFrameNumber, QVRCAMERA_MODE_BLOCKING, QVRCAMERA_MODE_NEWER_IF_AVAILABLE, &cameraPeerFrame);
            if (ret < 0) {
                log_error("%s::%s QVRCameraDevice_GetFrame peerFrameNumber=%d failed, ret=%d",__FUNCTION__, mSecCameraName, peerFrameNumber, ret);
                if (frameNumber > 0) {
                    QVRCameraDevice_ReleaseFrame(mCameraDevice, frameNumber);
                }
                usleep(5000);
                continue;
            }
        }
        log_info("%s:: get buffer len = %d", __FUNCTION__, cameraFrame.height * cameraFrame.width);
        int cam_w = cameraFrame.width;
        int cam_h = cameraFrame.height;
        if (isAutoExposure) {
            lastLongTs = cameraFrame.start_of_exposure_ts;
            if(mLongCallback) {
                mLongCallback(lastLongTs,(char *)cameraFrame.buffer,(char *)cameraPeerFrame.buffer,cam_w,cam_h);
            }
            if (peerFrameNumber > 0) {
                QVRCameraDevice_ReleaseFrame(mSecCameraDevice, peerFrameNumber);
            }
            if (frameNumber > 0) {
                QVRCameraDevice_ReleaseFrame(mCameraDevice, frameNumber);
            }
        } else {
            float long_twb[16] = {0};
            uint64_t long_ts = lastLongTs;
            uint64_t short_ts = cameraFrame.start_of_exposure_ts;
            if (peerFrameNumber > 0) {
                QVRCameraDevice_ReleaseFrame(mSecCameraDevice, peerFrameNumber);
            }
            if (frameNumber > 0) {
                QVRCameraDevice_ReleaseFrame(mCameraDevice, frameNumber);
            }
            qvrservice_head_tracking_data_t *framePose_l = nullptr;
            ret = QVRServiceClient_GetHistoricalHeadTrackingData(qvrHelper, &framePose_l, long_ts+ gQTimeToAndroidBoot);
            if (ret < 0) {
                log_error("%s:: QVRServiceClient_GetHistoricalHeadTrackingData  failed, ret=%d",__FUNCTION__, ret);
                continue;
            }
            toMatrix(framePose_l->rotation, framePose_l->translation, long_twb);
            float twb[16] = {0};
            qvrservice_head_tracking_data_t *framePose = nullptr;
            ret = QVRServiceClient_GetHistoricalHeadTrackingData(qvrHelper, &framePose, short_ts + gQTimeToAndroidBoot);
            log_info("QVRServiceClient_GetHistoricalHeadTrackingData %llu %llu",short_ts + gQTimeToAndroidBoot,framePose->ts);
            if (ret < 0) {
                log_error("%s:: QVRServiceClient_GetHistoricalHeadTrackingData  failed, ret=%d",__FUNCTION__, ret);
                continue;
            }
            toMatrix(framePose->rotation, framePose->translation, twb);
            log_info("%s:: pose is rotation = {%f, %f, %f, %f}, translation= {%f, %f, %f}",
                  __FUNCTION__, framePose->rotation[0], framePose->rotation[1], framePose->rotation[2], framePose->rotation[3],
                  framePose->translation[0], framePose->translation[1], framePose->translation[2]);
            if(mShortCallback) {
                mShortCallback(
                        short_ts,(char *) cameraFrame.buffer,(char *) cameraPeerFrame.buffer,twb, nullptr,framePose->prediction_coff_s,
                        long_ts,long_twb,nullptr,framePose_l->prediction_coff_s,cam_w,cam_h
                );
            }
        }
        lastFrameNumber = frameNumber;
    }
    log_debug("sa_status %s:: exit threadLoop", __FUNCTION__);
    pthread_exit(NULL);
}
int startQvrClient() {
    log_debug("sa_status startQvrClient");
    int32_t ret = -1;
    if (qvrHelper == nullptr) {
        log_error("%s:: qvrHelper is not inited!", __FUNCTION__);
        return -1;
    }
    do {
        if (start_qvr) {
            ret = QVRServiceClient_SetTrackingMode(qvrHelper, TRACKING_MODE_POSITIONAL);
            if (ret < 0) {
                log_error("%s:: QVRServiceClient_SetTrackingMode ret=%d",
                          __FUNCTION__, ret);
                break;
            }
        }
        mcCurrentServiceState = QVRServiceClient_GetVRMode(qvrHelper);
        const int maxTries = 1;
        const int waitTime = 500000;
        int attempt = 0;
        log_debug("%s:: mcCurrentServiceState is %d", __FUNCTION__, mcCurrentServiceState);
        while (((mcCurrentServiceState = QVRServiceClient_GetVRMode(qvrHelper)) != VRMODE_STARTED) && (attempt < maxTries)) {
            if (attempt > 0) {
                log_debug("%s:: called but VR service is in unexpected state, waiting... (attempt %d)", __FUNCTION__, attempt);
            }
            switch (mcCurrentServiceState) {
                case VRMODE_STOPPED:
                    log_debug("%s:: StartVRMode2 ...", __FUNCTION__);
                    if(start_qvr) {
                        ret = QVRServiceClient_StartVRMode(qvrHelper);
                        if (ret != QVR_SUCCESS) {
                            log_error("%s:: QVRServiceClient_StartVRMode, ret=%d",
                                    __FUNCTION__, ret);
                        }
                    }
                    break;
                case VRMODE_PAUSED:
                    log_debug("%s:: ResumeVRMode ...", __FUNCTION__);
                    if(start_qvr) {
                        ret = QVRServiceClient_ResumeVRMode(qvrHelper);
                        if (ret != QVR_SUCCESS) {
                            log_error("%s:: QVRServiceClient_ResumeVRMode ret=%d",
                                    __FUNCTION__, ret);
                            log_error("%s:: StopVRMode ...", __FUNCTION__);
                            ret = QVRServiceClient_StopVRMode(qvrHelper);
                            if (ret != QVR_SUCCESS) {
                                log_error(
                                        "%s:: QVRServiceClient_StopVRMode ret=%d",
                                        __FUNCTION__, ret);
                            }
                        }
                    }
                    break;
                case VRMODE_UNSUPPORTED:
                case VRMODE_STARTING:
                case VRMODE_STARTED:
                case VRMODE_STOPPING:
                case VRMODE_HEADLESS:
                case VRMODE_PAUSING:
                case VRMODE_RESUMING:
                default:
                    break;
            }
            //usleep(waitTime);
            log_debug("%s:: QvrServiceState is %s", __FUNCTION__, QVRServiceClient_StateToName(QVRServiceClient_GetVRMode(qvrHelper)));
            attempt++;
        }
    } while (false);
    if(mcCurrentServiceState == VRMODE_STARTED){
        ret = 0;
    }
    return ret;
}
int stopQvrClient() {
    int ret = -1;
    QVRSERVICE_VRMODE_STATE serviceState;
    serviceState = QVRServiceClient_GetVRMode(qvrHelper);
    log_debug("sa_status stopQvrClient %s:: QvrServiceState now is %s and before is %s", __FUNCTION__, QVRServiceClient_StateToName(serviceState), QVRServiceClient_StateToName(mcCurrentServiceState));
    const int maxTries = 1;
    const int waitTime = 500000;
    int attempt = 0;
    while (!(serviceState == VRMODE_UNSUPPORTED || serviceState == VRMODE_HEADLESS
             || serviceState == VRMODE_STOPPED) && (attempt < maxTries)) {
        if (attempt > 0)
            log_debug("%s:: called but VR service is in unexpected state, waiting... (attempt %d)", __FUNCTION__, attempt);
        switch (serviceState) {
            case VRMODE_STARTED:
                if (start_qvr) {
                    ret = QVRServiceClient_StopVRMode(qvrHelper);
                    if (ret < 0) {
                        log_error("%s:: QVRServiceClient_StopVRMode, ret=%d",
                                  __FUNCTION__,
                                  ret);
                    }
                }
                break;
            case VRMODE_PAUSED:
                log_debug("%s:: StopVRMode ...", __FUNCTION__);
                if(start_qvr){
                    ret = QVRServiceClient_StopVRMode(qvrHelper);
                    if (ret != QVR_SUCCESS) {
                        log_error("%s:: QVRServiceClient_StopVRMode, ret=%d", __FUNCTION__, ret);
                    }
                }
                break;
            case VRMODE_UNSUPPORTED:
            case VRMODE_STARTING:
            case VRMODE_STOPPING:
            case VRMODE_STOPPED:
            case VRMODE_HEADLESS:
                // When in headless mode, the gadget is disconnected. And the "serviceState"
                // is unknown. So we don't handle the situation in this case.
            case VRMODE_PAUSING:
            case VRMODE_RESUMING:
            default:
                break;
        }
        //usleep(waitTime);
        serviceState = QVRServiceClient_GetVRMode(qvrHelper);
        log_debug("%s:: QvrServiceState is %s", __FUNCTION__, QVRServiceClient_StateToName(serviceState));
        attempt++;
    }
    return ret;
}
int stopClient(bool withQvrClient) {
    log_debug("sa_status stopClient withQvrClient:%d clientStarted:%d",withQvrClient,clientStarted);
    if (!clientStarted) {
        return 0;
    }
    clientStarted = false;
    mThread.join();
    if (mCameraDevice != nullptr) {
        QVRCameraDevice_DetachCamera(mCameraDevice);
        mCameraDevice = nullptr;
    }
    if (mSecCameraDevice != nullptr) {
        QVRCameraDevice_DetachCamera(mSecCameraDevice);
        mSecCameraDevice = nullptr;
    }
    if(withQvrClient){
        stopQvrClient();
    }
    return 0;
}
int startClient(bool withqvrClient) {
    log_debug("sa_status startClient withqvrClient:%d clientStarted:%d startQvr=%d use_2cam=%d",withqvrClient,clientStarted,start_qvr,use_2cam);
    if(clientStarted){
        return 0;
    }
    int ret = 0;
    char prop[128] = {0};
    int res = __system_property_get("persist.debug_handshankstartqvr", prop);
    if (res > 0) {
        start_qvr = atoi(prop);
    }else {
        res = __system_property_get("debug_handshankstartqvr", prop);
        if(res > 0){
            start_qvr = atoi(prop);
        }
    }
    res = __system_property_get("persist.debug_handshankuse2cam", prop);
    if(res > 0) {
        use_2cam = atoi(prop);
    }
    if(withqvrClient){
        ret = startQvrClient();
    }
    if(ret < 0){
        return -1;
    }
    if (mCameraClient == nullptr) {
        log_error("%s:: QVRCameraClient_Create failed!", __FUNCTION__);
        return -1;
    }
    qvr_plugin_param_t params[2];
    params[0].name = QVR_CAMCLIENT_ATTACH_STRING_PREF_FORMAT;
    params[0].val = QVR_CAMDEVICE_FORMAT_DEFAULT;
    std::string val_format = "QVR_CAMDEVICE_FORMAT_DEFAULT";
    strcpy(mCameraName, "tracking");
    mCameraDevice = QVRCameraClient_AttachCameraWithParams(mCameraClient, mCameraName, params, 1);
    if (mCameraDevice == nullptr) {
        log_error("%s:: QVRCameraClient_AttachCameraWithParams with QVR_CAMDEVICE_FORMAT_RAW failed for %s", __FUNCTION__, mCameraName);
        return -1;
    } else {
        log_debug("%s:: QVRCameraClient_AttachCameraWithParams %s with QVR_CAMDEVICE_FORMAT_RAW OK", __FUNCTION__, mCameraName);
    }
    ret = 0;
    if (ret < 0) {
        log_error("%s:: %s QVRCameraDevice_Start failed!", __FUNCTION__, mCameraName);
        QVRCameraDevice_DetachCamera(mCameraDevice);
        return -1;
    }
    if(!use_2cam){
        strcpy(mSecCameraName, "ctrl-tracking");
        mSecCameraDevice = QVRCameraClient_AttachCameraWithParams(mCameraClient, mSecCameraName, params, 1);
        if (mSecCameraDevice == nullptr) {
            log_error("%s:: QVRCameraClient_AttachCameraWithParams with QVR_CAMDEVICE_FORMAT_RAW failed for %s", __FUNCTION__, mSecCameraName);
            QVRCameraDevice_DetachCamera(mCameraDevice);
            return -1;
        } else {
            log_debug("%s:: QVRCameraClient_AttachCameraWithParams %s with QVR_CAMDEVICE_FORMAT_RAW OK", __FUNCTION__, mSecCameraName);
        }
        ret = 0;
        if (ret < 0) {
            log_error("%s:: %s QVRCameraDevice_Start failed!", __FUNCTION__, mSecCameraName);
            QVRCameraDevice_DetachCamera(mSecCameraDevice);
            QVRCameraDevice_DetachCamera(mCameraDevice);
            return -1;
        }
    }
    clientStarted = true;
    mThread = std::thread(&threadLoop);
    log_debug("startClient finish");
    return 0;
}
int qvr_create(){
    log_debug("sa_status qvr_create %d",(mCameraClient != nullptr && qvrHelper != nullptr));
    if (mCameraClient != nullptr && qvrHelper != nullptr) {
        log_error("m_qvr_camera_client and m_qvr_service_client has created.");
        return 0;
    }
    mCameraClient = QVRCameraClient_Create();
	if(mCameraClient == nullptr){
        log_error("%s QVRCameraClient_Create failed",__FUNCTION__);
	}
    qvrHelper = QVRServiceClient_Create();
    QVRServiceClient_SetClientStatusCallback(qvrHelper,qvr_status_callback, nullptr);
	if(qvrHelper == nullptr){
        log_error("%s QVRServiceClient_Create failed",__FUNCTION__);
	}
    return 0;
}
int qvr_destory(){
    log_debug("sa_status qvr_destory");
    mLongCallback = nullptr;
    mShortCallback = nullptr;
    mPauseCallback = nullptr;
    mResumeCallback = nullptr;
	return 0;
}
int qvr_pasue(){
    log_debug("sa_status qvr_pasue");
    int res = stopClient(true);
    isResume = false;
    return res;
}
int qvr_resume(){
    log_debug("sa_status qvr_resume");
    int res = startClient(true);
    isResume = true;
    return res;
}
void qvr_setPauseCallback(std::function<void()> callback_) {
    mPauseCallback = callback_;
}
void qvr_setResumeCallback(std::function<void()> callback_) {
    mResumeCallback = callback_;
}
void qvr_setShortCallback(std::function<void(uint64_t ts,char *camdata1,char *camdata2,float * twb,
                                              float *linear_velocity,float *angular_velocity,
                                              uint64_t long_ts,float * l_twb,float *long_linear_velocity,
                                              float *long_angular_velocity,
                                              int w,int h)> callback_){
    mShortCallback = callback_;
}
void qvr_setLongCallback(std::function<void(uint64_t ts,char *camdata1,char *camdata2,int w,int h)> callback_){
    mLongCallback = callback_;
}
void qvr_need_camera(bool need){
    needCamera = need;
}
}