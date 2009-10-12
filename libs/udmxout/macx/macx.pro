include(../../../variables.pri)

TEMPLATE	= lib
LANGUAGE	= C++
TARGET		= udmxout

CONFIG          += plugin
INCLUDEPATH	+= ../../../libs ../../macx/libusb
LIBS		+= ../../macx/libusb/libusb.a -lIOKit

target.path 	= $$OUTPUTPLUGINDIR
INSTALLS	+= target

# uDMX headers
HEADERS += ../unix/udmxdevice.h \
	   ../unix/udmxout.h

# uDMX plugin sources
SOURCES += ../unix/udmxdevice.cpp \
	   ../unix/udmxout.cpp
