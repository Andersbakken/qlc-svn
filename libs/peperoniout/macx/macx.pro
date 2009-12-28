include(../../../variables.pri)

TEMPLATE	= lib
LANGUAGE	= C++
TARGET		= peperoniout

CONFIG          += plugin
INCLUDEPATH	+= ../../common
INCLUDEPATH	+= ../../macx/libusb
DEPENTPATH	+= ../unix
LIBS		+= ../../macx/libusb/libusb.a
LIBS		+= -framework IOKit -framework CoreFoundation

target.path 	= $$OUTPUTPLUGINDIR
INSTALLS	+= target

HEADERS += ../unix/peperonidevice.h \
	   ../unix/peperoniout.h

SOURCES += ../unix/peperonidevice.cpp \
	   ../unix/peperoniout.cpp
