include(../../variables.pri)

TEMPLATE 	= lib
LANGUAGE 	= C++
TARGET 		= hidinput

INCLUDEPATH	+= . ../../libs/
CONFIG          += plugin warn_on release

target.path	= $$INPUTPLUGINDIR
!macx:INSTALLS	+= target

macx:DESTDIR    = ../../main/qlc.app/Contents/Plugins/input

# Input
HEADERS += configurehidinput.h \
           hiddevice.h \
           hideventdevice.h \
           hidinput.h \
	   hidjsdevice.h \
           hidpoller.h

FORMS += configurehidinput.ui

SOURCES += configurehidinput.cpp \
           hiddevice.cpp \
           hideventdevice.cpp \
           hidinput.cpp \
	   hidjsdevice.cpp \
           hidpoller.cpp
