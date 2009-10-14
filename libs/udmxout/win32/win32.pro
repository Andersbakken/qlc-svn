include(../../../variables.pri)

TEMPLATE	= lib
LANGUAGE	= C++
TARGET		= udmxout

CONFIG          += plugin
INCLUDEPATH	+= ../../

target.path 	= $$OUTPUTPLUGINDIR
INSTALLS	+= target

# Headers
HEADERS += ../unix/udmxdevice.h \
	   ../unix/udmxout.h \
	   libusb_dyn.h

# Sources
SOURCES += ../unix/udmxdevice.cpp \
	   ../unix/udmxout.cpp \
	   libusb_dyn.c
