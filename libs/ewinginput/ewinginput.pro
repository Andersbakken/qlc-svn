include(../../variables.pri)

TEMPLATE 	= lib
LANGUAGE 	= C++
TARGET 		= ewinginput

INCLUDEPATH	+= . ../../libs/
CONFIG          += plugin
QT		+= network

target.path	= $$INPUTPLUGINDIR
!macx:INSTALLS	+= target

macx:DESTDIR    = ../../main/qlc.app/Contents/Plugins/input

# Input
HEADERS += ewinginput.h \
	   ewing.h

SOURCES += ewinginput.cpp \
	   ewing.cpp
