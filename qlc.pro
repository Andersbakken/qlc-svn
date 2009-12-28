include(variables.pri)

TEMPLATE	= subdirs

SUBDIRS		= libs
SUBDIRS		+= main
SUBDIRS		+= fixtures
SUBDIRS		+= inputprofiles
SUBDIRS		+= fixtureeditor
SUBDIRS		+= etc
SUBDIRS		+= docs
SUBDIRS		+= main/test

DEBIAN_CLEAN	+= debian/*.substvars debian/*.log debian/*.debhelper
DEBIAN_CLEAN	+= debian/files debian/dirs
QMAKE_CLEAN	+= $$DEBIAN_CLEAN
QMAKE_DISTCLEAN += $$DEBIAN_CLEAN
