include(../variables.pri)

TEMPLATE	= subdirs
TARGET		= icons

unix:desktop.path	= /usr/share/applications/
unix:desktop.files	+= qlc.desktop \
			   qlc-fixtureeditor.desktop

unix:icons.path		= /usr/share/pixmaps/
unix:icons.files	+= ../gfx/qlc.png \
			   ../gfx/qlc-fixtureeditor.png

unix:!macx:INSTALLS	+= icons desktop

# Win32 additional DLL installation
win32 {
	# Qt Libraries
        qtlibs.path  = $$LIBSDIR
        release:qtlibs.files =	$$(QTDIR)/bin/QtCore4.dll \
				$$(QTDIR)/bin/QtGui4.dll \
				$$(QTDIR)/bin/QtXml4.dll

	debug:qtlibs.files =	$$(QTDIR)/bin/QtCored4.dll \
				$$(QTDIR)/bin/QtGuid4.dll \
				$$(QTDIR)/bin/QtXmld4.dll

	# MinGW library
	mingw.path      = $$LIBSDIR
	mingw.files     = $$(SystemDrive)/MinGW/bin/mingwm10.dll

	# NullSoft installer files
	nsis.path	= $$DATADIR\nsis
	nsis.files	= qlc.nsi \
			  qlc-input-midi.nsi \
			  qlc-input-enttec-wing.nsi \
			  qlc-output-enttecdmxusb.nsi \
			  qlc-output-enttecdmxusbpro.nsi \
			  qlc-output-midi.nsi \
			  qlc-output-udmx.nsi \
			  qlc-output-peperoni.nsi

        INSTALLS        += qtlibs mingw nsis
}
