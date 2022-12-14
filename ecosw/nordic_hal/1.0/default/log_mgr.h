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

#pragma once
#include <string>
#include <fstream>
#include <android/log.h>
namespace sctrl {
#define ENABLE_LOG                                  1
#if ENABLE_LOG
#ifndef LOG_TAG
#define LOG_TAG "KXControllerHal"
#endif
#define log_debug(...)    //__android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define log_info(...)     //__android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define log_warn(...)     //__android_log_print(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
#define log_error(...)    __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#else
#define log_debug(...)  ((void)0)
#define log_info(...)  ((void)0)
#define log_warn(...)  ((void)0)
#define log_error(...)  ((void)0)
#endif
}
