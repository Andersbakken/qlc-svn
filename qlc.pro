TEMPLATE	= subdirs

SUBDIRS		= libs
SUBDIRS		+= main
SUBDIRS		+= fixtures
SUBDIRS		+= inputtemplates
SUBDIRS		+= fixtureeditor
SUBDIRS		+= etc

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
	qtlibs.path	= $$(SystemDrive)/QLC/
	qtlibs.files	= $$(QTDIR)/bin/QtCore4.dll \
			  $$(QTDIR)/bin/QtGui4.dll \
			  $$(QTDIR)/bin/QtXml4.dll

	# TODO: MinGW might be elsewhere, but there's no easy way to find it
	mingw.path	= $$(SystemDrive)/QLC/
	mingw.files	= $$(SystemDrive)/MinGW/bin/mingwm10.dll

	INSTALLS	+= qtlibs mingw
}