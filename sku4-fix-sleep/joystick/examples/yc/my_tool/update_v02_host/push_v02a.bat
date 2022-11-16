adb wait-for-device
adb root
adb wait-for-device
adb remount

adb push v02a data/
adb shell  "chmod 777 data/v02a"
adb push  fw_v02a_v2.zip data/fw.zip
adb shell  "/data/v02a xxx  /data/fw.zip"

@pause