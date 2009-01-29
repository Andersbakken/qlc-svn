Name "MIDI input plugin for Q Light Controller"
OutFile "qlc-input-midi-3.0.exe"
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
  File /oname=Plugins\Input\midiinput.dll Plugins\Input\midiinput.dll
SectionEnd
