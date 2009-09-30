Name "Peperoni USBDMX output plugin for Q Light Controller"
OutFile "qlc-output-usbdmx-3.0.2.exe"
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
  File /oname=Plugins\Output\usbdmxout.dll ..\Plugins\Output\usbdmxout.dll
SectionEnd
