include(../../../variables.pri)

TEMPLATE	= lib
LANGUAGE	= C++
TARGET		= peperoniout

CONFIG          += plugin
INCLUDEPATH	+= ../../common
CONFIG		+= link_pkgconfig
PKGCONFIG 	+= libusb
DEPENDPATH	+= ../unix

HEADERS += ../unix/peperonidevice.h \
	   ../unix/peperoniout.h

SOURCES += ../unix/peperonidevice.cpp \
	   ../unix/peperoniout.cpp

target.path 	= $$INSTALLROOT/$$OUTPUTPLUGINDIR
INSTALLS	+= target
