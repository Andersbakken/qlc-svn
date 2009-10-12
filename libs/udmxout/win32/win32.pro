include(../../../variables.pri)

TEMPLATE	= lib
LANGUAGE	= C++
TARGET		= udmxout

CONFIG          += plugin
INCLUDEPATH	+= ../../../libs/

target.path 	= $$OUTPUTPLUGINDIR
INSTALLS	+= target

# Headers
HEADERS += ../unix/configureudmxout.h \
	   ../unix/udmxdevice.h \
	   ../unix/udmxout.h \
	   libusb_dyn.h

# Sources
SOURCES += ../unix/configureudmxout.cpp \
	   ../unix/udmxdevice.cpp \
	   ../unix/udmxout.cpp \
	   libusb_dyn.cpp
