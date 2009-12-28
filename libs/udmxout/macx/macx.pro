include(../../../variables.pri)

TEMPLATE	= lib
LANGUAGE	= C++
TARGET		= udmxout

CONFIG          += plugin
INCLUDEPATH	+= ../../common
INCLUDEPATH	+= ../../macx/libusb
DEPENDPATH	+= ../unix
LIBS		+= ../../macx/libusb/libusb.a
LIBS		+= -framework IOKit -framework CoreFoundation

target.path 	= $$OUTPUTPLUGINDIR
INSTALLS	+= target

# uDMX headers
HEADERS += ../unix/udmxdevice.h \
	   ../unix/udmxout.h

# uDMX plugin sources
SOURCES += ../unix/udmxdevice.cpp \
	   ../unix/udmxout.cpp
