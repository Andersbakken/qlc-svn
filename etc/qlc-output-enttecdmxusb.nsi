Name "Enttec DMX USB output plugin for Q Light Controller"
OutFile "qlc-output-enttecdmxusb-3.0.3.exe"
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
  File /oname=Plugins\Output\enttecdmxusbout.dll ..\Plugins\Output\enttecdmxusbout.dll
SectionEnd
