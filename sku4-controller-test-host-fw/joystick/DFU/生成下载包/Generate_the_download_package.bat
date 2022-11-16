echo please version
set /P version=Enter: %=%
echo %version%

nrfutil pkg generate --hw-version 52 --sd-req 0xB7 --application-version 0xff --application qcom_sku4_master_nordic.hex --key-file qcom_sku4_priv.pem qcom_sku4_application_DFU_v%version%.zip
pause