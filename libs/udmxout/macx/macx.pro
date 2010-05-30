include(../../../variables.pri)

TEMPLATE	= lib
LANGUAGE	= C++
TARGET		= udmxout

CONFIG          += plugin
INCLUDEPATH	+= ../../common
CONFIG		+= link_pkgconfig
PKGCONFIG	+= libusb
DEPENDPATH	+= ../unix

HEADERS += ../unix/udmxdevice.h \
	   ../unix/udmxout.h

SOURCES += ../unix/udmxdevice.cpp \
	   ../unix/udmxout.cpp

target.path 	= $$INSTALLROOT/$$OUTPUTPLUGINDIR
INSTALLS	+= target
