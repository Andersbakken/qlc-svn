include(../../variables.pri)

TEMPLATE 	= lib
LANGUAGE 	= C++
TARGET 		= ewinginput

INCLUDEPATH	+= ../../libs/common
CONFIG          += plugin
QT		+= network

target.path	= $$INPUTPLUGINDIR
INSTALLS	+= target

win32 {
	# Qt Libraries
	qtnetwork.path  = $$LIBSDIR
	CONFIG(release, debug|release) qtnetwork.files = $$(QTDIR)/bin/QtNetwork4.dll
	CONFIG(debug, debug|release) qtnetwork.files = $$(QTDIR)/bin/QtNetworkd4.dll
	INSTALLS	+= qtnetwork
}

# Input
HEADERS += ewinginput.h \
	   eplaybackwing.h \
	   eshortcutwing.h \
	   eprogramwing.h \
	   ewing.h

SOURCES += ewinginput.cpp \
	   eplaybackwing.cpp \
	   eshortcutwing.cpp \
	   eprogramwing.cpp \
	   ewing.cpp
