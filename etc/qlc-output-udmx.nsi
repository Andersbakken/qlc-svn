Name "Anyma uDMX output plugin for Q Light Controller"
OutFile "qlc-output-udmx-3.0.4.exe"
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
  File /oname=Plugins\Output\udmxout.dll ..\Plugins\Output\udmxout.dll
SectionEnd
