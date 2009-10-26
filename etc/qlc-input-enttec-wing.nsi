Name "ENTTEC wing input plugin for Q Light Controller"
OutFile "qlc-input-enttec-wing-3.0.4.exe"
InstallDir C:\QLC
RequestExecutionLevel user
;--------------------------------
; Pages
Page directory
Page instfiles
;--------------------------------
Section ""
  SetOutPath $INSTDIR
  CreateDirectory $INSTDIR\Plugins\Input
  File /oname=Plugins\Input\ewinginput.dll ..\Plugins\Input\ewinginput.dll
  File ..\QtNetwork4.dll
SectionEnd
