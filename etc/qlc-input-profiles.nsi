Name "Q Light Controller Input Profiles"
OutFile "qlc-input-profiles.exe"
InstallDir C:\QLC
RequestExecutionLevel user
;--------------------------------
; Pages
Page directory
Page instfiles
;--------------------------------
Section ""
  SetOutPath $INSTDIR
  File /r ..\InputProfiles
SectionEnd
