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

package com.kineticsxr.service.nordic.bridge;
import android.content.Context;
import android.content.Intent;
import android.os.ParcelFileDescriptor;
import java.io.IOException;
public class HalNordicJni {
    static {
        System.loadLibrary("_hal_nordic_jni");
    }
    private String cache_path;
    private Context mContext;
    private String data_path;
    public int fds[];
    public HalNordicJni(Context context) {
        mContext = context;
        cache_path = context.getExternalCacheDir().getAbsolutePath();
        data_path = context.getDataDir().getAbsolutePath();
        LogUtils.LOGD("HalNordicJni cache_path:"+cache_path);
        LogUtils.LOGD("              data_path:"+cache_path);
    }
    public void create() {
        fds = new int[]{-1, 0, -1, 0};
        nativeCreate();
		int fdSize[] = new int[]{0};
		fdSize[0] = -1;
		int[] data = new int[2];
		nativeGetMem(data);
		fdSize[0] = data[0];
		try {
			ParcelFileDescriptor pdf = ParcelFileDescriptor.fromFd(data[1]);
			if(pdf != null){
                fds[0] = pdf.getFd();
                fds[1] = fdSize[0];
	            LogUtils.LOGD("HalNordicJni create fds[0-3]" +fds[0]+" "+fds[1]+" "+fds[2]+" "+fds[3]);
			/*
				int fd_dup = nativedup(fds[0]);
	            LogUtils.LOGD("mNordicService.hal_nordic_client_get_memory " + fds[0] + " "+fds[1] + " " + fd_dup);
	            fds[0] = fd_dup;
	            */
            }
		} catch (IOException e) {
			e.printStackTrace();
			fdSize[0] = -1;
		}
    }
    public void destroy() {
        nativeDestroy();
    }
    public void resume() {
        LogUtils.LOGD("resume");
		nativeStart();
    }
    public void pause() {
        LogUtils.LOGD("pause");
   		nativeStop();
    }
    public boolean performHapticFeedback(long durationNs, float frequency, float amplitude, int type) {
        return nativeSetVibration(durationNs, frequency, amplitude, type) >= 0;
    }
    public native int nativeCreate();
    public native int nativeDestroy();
    public native int nativeStart();
    public native int nativeStop();
    public native void nativeGetMem(int[] data);
    public native int nativeSetVibration(long durationNs, float frequency, float amplitude, int type);
    public native float nativeGetNordicVersion();
    public native float nativeGetControllerVersion(int type);
    public native int nativeBindController(int type);
    public native int nativeUnbindController(int type);
    public native int nativeCancelBindController();
    public native int nativeGetBindControllerState();
}
