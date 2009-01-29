Name "Q Light Controller"
OutFile "qlc-3.0.exe"
InstallDir C:\QLC
RequestExecutionLevel user
;--------------------------------
; Pages
Page directory
Page instfiles
;--------------------------------
Section ""
  SetOutPath $INSTDIR
  File mingwm10.dll
  File qlccommon3.dll
  File qlc.exe
  File qlc-fixtureeditor.exe
  File QtCore4.dll
  File QtGui4.dll
  File QtXml4.dll
  File /r Documents
  File /r Fixtures
  File /r InputDevices
SectionEnd
