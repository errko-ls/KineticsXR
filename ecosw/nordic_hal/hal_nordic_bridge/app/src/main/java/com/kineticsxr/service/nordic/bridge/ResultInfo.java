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
import android.os.Parcel;
import android.os.ParcelFileDescriptor;
import android.os.Parcelable;
import java.io.FileDescriptor;
import java.io.IOException;
public class ResultInfo implements Parcelable {
    public int imu_fd;
    public int imu_fd_size;
    public int pose_fd;
    public int pose_fd_size;
    public String client_so_path = "";
    public String version_str = "";
    public ResultInfo() {
    }
    protected ResultInfo(Parcel in) {
        if (in.readInt() == 1) {
            imu_fd = in.readFileDescriptor().getFd();
            imu_fd_size = in.readInt();
        } else {
            imu_fd = -1;
            imu_fd_size = 0;
        }
        if (in.readInt() == 1) {
            pose_fd = in.readFileDescriptor().getFd();
            pose_fd_size = in.readInt();
        } else {
            pose_fd = -1;
            pose_fd_size = 0;
        }

        client_so_path = in.readString();
        version_str = in.readString();
    }
    @Override
    public void writeToParcel(Parcel dest, int flags) {
        if (imu_fd > 0) {
            try {
                FileDescriptor fd = ParcelFileDescriptor.fromFd(imu_fd).getFileDescriptor();
                if (fd != null) {
                    dest.writeInt(1);
                    dest.writeFileDescriptor(fd);
                    dest.writeInt(imu_fd_size);
                } else {
                    dest.writeInt(0);
                }
            } catch (IOException io) {
                dest.writeInt(0);
            }
        } else {
            dest.writeInt(0);
        }
        if (pose_fd > 0) {
            try {
                FileDescriptor fd = ParcelFileDescriptor.fromFd(pose_fd).getFileDescriptor();
                if (fd != null) {
                    dest.writeInt(1);
                    dest.writeFileDescriptor(fd);
                    dest.writeInt(pose_fd_size);
                } else {
                    dest.writeInt(0);
                }
            } catch (IOException io) {
                dest.writeInt(0);
            }
        } else {
            dest.writeInt(0);
        }
        dest.writeString(client_so_path);
        dest.writeString(version_str);
    }
    @Override
    public int describeContents() {
        return 0;
    }
    public static final Creator<ResultInfo> CREATOR = new Creator<ResultInfo>() {
        @Override
        public ResultInfo createFromParcel(Parcel in) {
            return new ResultInfo(in);
        }
        @Override
        public ResultInfo[] newArray(int size) {
            return new ResultInfo[size];
        }
    };
}
