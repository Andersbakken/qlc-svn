include(../../../variables.pri)

TEMPLATE	= lib
LANGUAGE	= C++
TARGET		= peperoniout

CONFIG          += plugin
INCLUDEPATH	+= ../../ ../../macx/libusb
LIBS		+= ../../macx/libusb/libusb.a -lIOKit

target.path 	= $$OUTPUTPLUGINDIR
INSTALLS	+= target

HEADERS += ../unix/peperonidevice.h \
	   ../unix/peperoniout.h

SOURCES += ../unix/peperonidevice.cpp \
	   ../unix/peperoniout.cpp
