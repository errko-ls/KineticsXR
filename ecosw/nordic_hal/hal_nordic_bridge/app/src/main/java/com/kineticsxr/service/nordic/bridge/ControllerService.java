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
import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.os.ParcelFileDescriptor;
import android.os.RemoteException;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import java.io.IOException;
import com.kineticsxr.service.nordic.bridge.IClient;
import com.kineticsxr.service.nordic.bridge.IControllerInterface;
import com.kineticsxr.service.nordic.bridge.ResultInfo;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Message;
import android.os.RemoteException;
import android.os.RemoteCallbackList;
import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
public class ControllerService extends Service {
    private HalNordicJni halNordicJni;
    private RemoteCallbackList<IClient> mClients;
    private HandlerThread mThread;
    private Handler mHander;
    private boolean isResume = false;
    @Override
    public void onCreate() {
        super.onCreate();
        mClients = new RemoteCallbackList<IClient>() {
            @Override
            public void onCallbackDied(IClient callback) {
                super.onCallbackDied(callback);
                onClientChange();
            }
        };
        LogUtils.LOGD("onCreate");
        Intent nIntent = new Intent(this, ControllerService.class);
        PendingIntent pendingIntent = PendingIntent.getActivity(this, 0, nIntent, PendingIntent.FLAG_IMMUTABLE);
        NotificationManager nm = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
        NotificationChannel nc = new NotificationChannel("ControllerService_Id", "ControllerService", NotificationManager.IMPORTANCE_MIN);
        nm.createNotificationChannel(nc);
        Notification.Builder mBuilder = new Notification.Builder(this)
                .setSmallIcon(R.mipmap.ic_launcher)
                .setContentTitle("ControllerService").setContentText("ControllerService running")
                .setContentIntent(pendingIntent).setChannelId("ControllerService_Id");
        startForeground(1, mBuilder.build());
        mThread = new HandlerThread("ControllerService_Handle");
        mThread.start();
        mHander = new Handler(mThread.getLooper()) {
            @Override
            public void handleMessage(@NonNull Message msg) {
                switch (msg.what) {

                }
                super.handleMessage(msg);
            }
        };
        halNordicJni = new HalNordicJni(this);
        halNordicJni.create();
    }
    @Override
    public void onDestroy() {
        mThread.quitSafely();
        halNordicJni.destroy();
        LogUtils.LOGD("ControllerService onDestroy");
        mClients.kill();
        super.onDestroy();
    }
    private Runnable clientChangeRunalbe = new Runnable() {
        @Override
        public void run() {
            int clientSize = mClients.getRegisteredCallbackCount();
            LogUtils.LOGD("onClientChange " + clientSize+" ; isResume:"+isResume);
            if (!isResume && clientSize > 0) {
                isResume = true;
                halNordicJni.resume();
            } else if (isResume && clientSize == 0) {
                halNordicJni.pause();
                isResume = false;
            }
        }
    };
    void onClientChange() {
        if (mHander.hasCallbacks(clientChangeRunalbe)) {
            mHander.removeCallbacks(clientChangeRunalbe);
        }
        mHander.post(clientChangeRunalbe);
    }
    IControllerInterface.Stub mStub = new IControllerInterface.Stub() {
		@Override
		public ResultInfo getShareMemoryInfo() throws RemoteException {
			ResultInfo info = new ResultInfo();
			LogUtils.LOGD("getShareMemoryInfo");
			info.imu_fd = halNordicJni.fds[0];
			info.imu_fd_size = halNordicJni.fds[1];
			info.pose_fd = halNordicJni.fds[2];
			info.pose_fd_size = halNordicJni.fds[3];
			
			LogUtils.LOGD("getShareMemoryInfo imu:"+info.imu_fd+" "+info.imu_fd_size+" pose:"+info.pose_fd+" "+info.pose_fd_size);
			return info;
		}
		@Override
		public void clientConnect(IClient client) throws RemoteException {
			LogUtils.LOGD("clientConnect "+client);
			mClients.register(client);
			onClientChange();
		}
		@Override
		public void clientDisconnect(IClient client) throws RemoteException {
			LogUtils.LOGD("clientDisconnect "+client);
			mClients.unregister(client);
			onClientChange();
		}
		@Override
		public boolean performHapticFeedback(long durationNs, float frequency, float amplitude, int type) throws RemoteException {
			if (halNordicJni != null) {
				return halNordicJni.performHapticFeedback(durationNs, frequency, amplitude, type);
			}
			return false;
		}
        @Override
        public float getNordicVersion() throws RemoteException {
            if (halNordicJni != null) {
                return halNordicJni.nativeGetNordicVersion();
            }
            return -1;
        }
        @Override
        public float getControllerVersion(int type) throws RemoteException {
            if (halNordicJni != null) {
                return halNordicJni.nativeGetControllerVersion(type);
            }
            return -1;
        }
        @Override
        public int bindController(int type) throws RemoteException {
            if (halNordicJni != null) {
                return halNordicJni.nativeBindController(type);
            }
            return -1;
        }
        @Override
        public int unbindController(int type) throws RemoteException {
            if (halNordicJni != null) {
                return halNordicJni.nativeUnbindController(type);
            }
            return -1;
        }
        @Override
        public int cancelBindController() throws RemoteException {
            if (halNordicJni != null) {
                return halNordicJni.nativeCancelBindController();
            }
            return -1;
        }
        @Override
        public int getControllerBindState() throws RemoteException {
            if (halNordicJni != null) {
                return halNordicJni.nativeGetBindControllerState();
            }
            return -1;
        }
    };
    @Nullable
    @Override
    public IBinder onBind(Intent intent) {
		LogUtils.LOGD("onBind " + (intent==null?" null":intent.toString()));
        return mStub;
    }
    @Override
    public boolean onUnbind(Intent intent) {
		LogUtils.LOGD("onUnbind " + (intent==null?" null":intent.toString()));
        return true;
    }
}
