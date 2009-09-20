Name "FTDIDMX output plugin for Q Light Controller"
OutFile "qlc-output-ftdidmx-3.0.2.exe"
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
  File /oname=Plugins\Output\ftdidmxout.dll ..\Plugins\Output\ftdidmxout.dll
  File /oname=ftd2xx.dll ..\ftd2xx.dll
SectionEnd
