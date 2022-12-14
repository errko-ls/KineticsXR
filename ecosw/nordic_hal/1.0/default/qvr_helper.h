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

#ifndef SCHANDSHANKSERVICE_QVR_HELPER_H
#define SCHANDSHANKSERVICE_QVR_HELPER_H
#include "functional"
namespace SC{
    int qvr_create();
    int qvr_destory();
    int qvr_pasue();
    int qvr_resume();
    void qvr_need_camera(bool need_camera);
    void qvr_setPauseCallback(std::function<void()> callback_);
    void qvr_setResumeCallback(std::function<void()> callback_);
    void qvr_setShortCallback(std::function<void(uint64_t ts,char *camdata1,char *camdata2,float * twb,
                                                  float *linear_velocity,float *angular_velocity,
                                                  uint64_t long_ts,float * l_twb,float *long_linear_velocity,
                                                  float *long_angular_velocity,
                                                  int w,int h)> callback_);
    void qvr_setLongCallback(std::function<void(uint64_t ts,char *camdata1,char *camdata2,int w,int h)> callback_);
}
#endif //SCHANDSHANKSERVICE_QVR_HELPER_H
