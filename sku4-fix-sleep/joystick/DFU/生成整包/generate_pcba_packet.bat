@echo "input version of application image"
@set /p version=version:
echo %version%

nrfutil settings generate --family NRF52 --application qcom_sku4_master_nordic.hex --application-version 1 --bootloader-version 1 --bl-settings-version 2 settings.hex
mergehex --merge qcom_sku4_masterNordic_bootloader.hex settings.hex --output bl_temp.hex
mergehex --merge bl_temp.hex  qcom_sku4_master_nordic.hex s132_nrf52_6.1.1_softdevice.hex --output qcom_sku4_pcba_v%version%.hex

@echo off
del %~dp0bl_temp.hex settings.hex /f /s /q >nul

pause