include(../../../variables.pri)

TEMPLATE	= lib
LANGUAGE	= C++
TARGET		= peperoniout

CONFIG          += plugin
INCLUDEPATH	+= ../.. ../../macx/libusb
LIBS		+= ../../macx/libusb/libusb.a -lIOKit

target.path 	= $$OUTPUTPLUGINDIR
INSTALLS	+= target

HEADERS += peperonidevice.h \
	   peperoniout.h

SOURCES += peperonidevice.cpp \
	   peperoniout.cpp
