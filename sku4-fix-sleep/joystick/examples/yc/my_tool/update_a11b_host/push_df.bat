adb wait-for-device
adb root
adb wait-for-device
adb remount

adb push a11b data/
adb shell  "chmod 777 data/a11b"
adb push  fw.zip data/
adb shell  "/data/a11b xxx  /data/fw.zip"

@pause