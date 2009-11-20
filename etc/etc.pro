include(../variables.pri)

TEMPLATE	= subdirs
TARGET		= icons

# Linux
unix:!macx {
	desktop.path	= /usr/share/applications/
	desktop.files	+= qlc.desktop \
			   qlc-fixtureeditor.desktop

	icons.path	= /usr/share/pixmaps/
	icons.files	+= ../gfx/qlc.png \
			   ../gfx/qlc-fixtureeditor.png

	INSTALLS	+= icons desktop
}

# Mac OSX
macx {
	icons.path	= $$DATADIR
	icons.files	+= ../gfx/qlc.icns

	plist.path	= $$DATADIR/..
	plist.files	+= Info.plist

	INSTALLS	+= icons plist
}

# Windows
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

samples.files += Sample.qxw
samples.path = $$DATADIR
INSTALLS += samples
