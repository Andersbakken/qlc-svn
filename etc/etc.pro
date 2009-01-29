include(../variables.pri)

TEMPLATE	= subdirs
TARGET		= icons

unix:desktop.path	= /usr/share/applications/
unix:desktop.files	+= qlc.desktop \
			   qlc-fixtureeditor.desktop

unix:icons.path		= /usr/share/pixmaps/
unix:icons.files	+= ../gfx/qlc.png \
			   ../gfx/qlc-fixtureeditor.png

unix:INSTALLS		+= icons desktop

# MAC installation setup
macx {
        system(rm -Rf main/qlc.app/Contents/docs)
        system(mkdir -p main/qlc.app/Contents/)
        system(cp -r docs main/qlc.app/Contents/)
        system(rm -Rf fixtureeditor/qlc-fixtureeditor.app/Contents/docs)
        system(mkdir -p fixtureeditor/qlc-fixtureeditor.app/Contents/)
        system(cp -r docs fixtureeditor/qlc-fixtureeditor.app/Contents/)
}

# Win32 additional DLL installation
win32 {
	# Qt Libraries
        qtlibs.path     = $$LIBSDIR
        qtlibs.files    = $$(QTDIR)/bin/QtCore4.dll \
                          $$(QTDIR)/bin/QtGui4.dll \
                          $$(QTDIR)/bin/QtXml4.dll

	# MinGW library
        mingw.path      = $$LIBSDIR
        mingw.files     = $$(SystemDrive)/MinGW/bin/mingwm10.dll

	# NullSoft installer files
	nsis.path	= $$DATADIR
	nsis.files	= qlc.nsi \
			  qlc-input-midi.nsi \
			  qlc-output-ftdidmx.nsi \
			  qlc-output-midi.nsi \
			  qlc-output-udmx.nsi \
			  qlc-output-usbdmx.nsi

        INSTALLS        += qtlibs mingw nsis
}
