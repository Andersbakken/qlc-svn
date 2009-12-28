include(../../variables.pri)

TEMPLATE 	= lib
LANGUAGE 	= C++
TARGET 		= hidinput

INCLUDEPATH	+= ../../libs/common
CONFIG          += plugin

target.path	= $$INPUTPLUGINDIR
INSTALLS	+= target

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
