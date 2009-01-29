Name "MIDI output plugin for Q Light Controller"
OutFile "qlc-output-midi-3.0.exe"
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
  File /oname=Plugins\Output\midiout.dll Plugins\Output\midiout.dll
SectionEnd
