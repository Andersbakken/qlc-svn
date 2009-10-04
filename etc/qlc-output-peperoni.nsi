Name "Peperoni USBDMX output plugin for Q Light Controller"
OutFile "qlc-output-peperoni-3.0.2.exe"
InstallDir C:\QLC
RequestExecutionLevel user
;--------------------------------
; Pages
Page directory
Page instfiles
;--------------------------------
Section ""
  SetOutPath $INSTDIR
  CreateDirectory $INSTDIR\Plugins\Output
  File /oname=Plugins\Output\peperoniout.dll ..\Plugins\Output\peperoniout.dll
  IfFileExists "Plugins\Output\usbdmxout.dll" RemoveUSBDMX PastRemoveUSBDMX

  RemoveUSBDMX:
   Delete "$INSTDIR\Plugins\Output\usbdmxout.dll"

  PastRemoveUSBDMX:
SectionEnd
