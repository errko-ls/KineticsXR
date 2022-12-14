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
#include <vendor/kineticsxr/hardware/nordic/1.0/INordic.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <hidlmemory/mapping.h>
#include <android/hidl/allocator/1.0/IAllocator.h>
#include <android/hidl/memory/1.0/IMemory.h>
namespace vendor::kineticsxr::hardware::nordic::implementation {
using ::android::hidl::memory::V1_0::IMemory;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;
struct Nordic : public V1_0::INordic {
public:
    Nordic();
    ~Nordic();
    // Methods from ::vendor::kineticsxr::hardware::nordic::V1_0::INordic follow.
    Return<void> helloWorld(const hidl_string &name, helloWorld_cb _hidl_cb) override;
    Return <int32_t> Nordic_Create() override;
    Return <int32_t> Nordic_Destroy() override;
    Return <int32_t> Nordic_Start() override;
    Return<void> Nordic_Get_Memory(Nordic_Get_Memory_cb _hidl_cb) override;
    Return <int32_t> Nordic_Stop() override;
    Return<float> Nordic_Get_Nordic_Version() override;
    Return<float> Nordic_Get_Controller_Version(int32_t lr) override;
    Return <int32_t> Nordic_Bind_Controller(int32_t lr) override;
    Return <int32_t> Nordic_Unbind_Controller(int32_t lr) override;
    Return <int32_t> Nordic_Cancel_Bind() override;
    Return <int32_t> Nordic_Get_Bind_State() override;
    Return <int32_t> Nordic_Set_Vibration(int32_t value) override;
    Return <int32_t> Nordic_Enter_Dfu() override;
    // Methods from ::android::hidl::base::V1_0::IBase follow.
private:
    native_handle_t *m_hidl_handle = nullptr;
    hidl_memory m_hidl_heap;
    void *m_hidl_heapMemData;
    sp <IMemory> m_hidl_heapMemory;
    int32_t mSize = 0;
};
// FIXME: most likely delete, this is only for passthrough implementations
extern "C" V1_0::INordic *HIDL_FETCH_INordic(const char *name);
}  // namespace vendor::kineticsxr::hardware::nordic::implementation
