Name "Q Light Controller Fixture Definitions"
OutFile "qlc-fixtures.exe"
InstallDir C:\QLC
RequestExecutionLevel user
;--------------------------------
; Pages
Page directory
Page instfiles
;--------------------------------
Section ""
  SetOutPath $INSTDIR
  File /r Fixtures
SectionEnd
