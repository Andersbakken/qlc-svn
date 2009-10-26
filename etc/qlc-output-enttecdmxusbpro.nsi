Name "Enttec DMX USB Pro (VCP) output plugin for Q Light Controller"
OutFile "qlc-output-enttecdmxusbpro-3.0.4.exe"
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
  File /oname=Plugins\Output\enttecdmxusbproout.dll ..\Plugins\Output\enttecdmxusbproout.dll
SectionEnd
