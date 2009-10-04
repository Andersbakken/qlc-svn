Name "Q Light Controller"
OutFile "qlc-3.0.3.exe"
InstallDir C:\QLC
RequestExecutionLevel user
;--------------------------------
; Pages
Page directory

Page custom StartMenuGroupSelect "" ": Start Menu Folder"
Function StartMenuGroupSelect
	Push $R1

	StartMenu::Select /checknoshortcuts "Don't create a start menu folder" /autoadd /lastused $R0 "Q Light Controller"
	Pop $R1

	StrCmp $R1 "success" success
	StrCmp $R1 "cancel" done
		; error
		MessageBox MB_OK $R1
		StrCpy $R0 "Q Light Controller" # use default
		Return
	success:
	Pop $R0

	done:
	Pop $R1
FunctionEnd

Page instfiles
Section
	SetOutPath $INSTDIR

	# this part is only necessary if you used /checknoshortcuts
	StrCpy $R1 $R0 1
	StrCmp $R1 ">" skip
		CreateDirectory $SMPROGRAMS\$R0
		CreateShortCut '$SMPROGRAMS\$R0\Q Light Controller.lnk' $INSTDIR\qlc.exe

		CreateDirectory $SMPROGRAMS\$R0
		CreateShortCut '$SMPROGRAMS\$R0\Fixture Definition Editor.lnk' $INSTDIR\qlc-fixtureeditor.exe

	skip:
SectionEnd

;--------------------------------
Section ""
  File ..\mingwm10.dll
  File ..\qlccommon.dll
  File ..\qlc.exe
  File ..\qlc-fixtureeditor.exe
  File ..\QtCore4.dll
  File ..\QtGui4.dll
  File ..\QtXml4.dll
  File /r ..\Documents
SectionEnd
