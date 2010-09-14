include(../../../variables.pri)

TEMPLATE	= lib
LANGUAGE	= C++
TARGET		= udmxout

CONFIG          += plugin
INCLUDEPATH	+= ../../interfaces
CONFIG		+= link_pkgconfig
PKGCONFIG	+= libusb
DEPENDPATH	+= ../unix

HEADERS += ../unix/udmxdevice.h \
	   ../unix/udmxout.h

SOURCES += ../unix/udmxdevice.cpp \
	   ../unix/udmxout.cpp

# This must be after "TARGET = " and before target installation so that
# install_name_tool can be run before target installation
include(../../../macx/nametool.pri)

target.path 	= $$INSTALLROOT/$$OUTPUTPLUGINDIR
INSTALLS	+= target
