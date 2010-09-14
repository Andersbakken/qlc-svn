include(variables.pri)

TEMPLATE	= subdirs

SUBDIRS		+= engine
SUBDIRS		+= ui
SUBDIRS		+= fixtures
SUBDIRS		+= inputprofiles
SUBDIRS		+= fixtureeditor
SUBDIRS		+= etc
SUBDIRS		+= docs
macx:SUBDIRS	+= launcher
SUBDIRS		+= plugins

unix:!macx:DEBIAN_CLEAN	+= debian/*.substvars debian/*.log debian/*.debhelper
unix:!macx:DEBIAN_CLEAN	+= debian/files debian/dirs
unix:!macx:QMAKE_CLEAN	+= $$DEBIAN_CLEAN
unix:!macx:QMAKE_DISTCLEAN += $$DEBIAN_CLEAN

unittests.target = test
QMAKE_EXTRA_TARGETS += unittests

unix {
	unittests.commands += ./unittest.sh
}

win32 {
	# *sigh*.. Mr Ballmer's shell is just so utterly depressing
	debug:unittests.commands += unittest.bat debug
	release:unittests.commands += unittest.bat release
}

# Leave this on the last row of this file
macx:SUBDIRS	+= macx
