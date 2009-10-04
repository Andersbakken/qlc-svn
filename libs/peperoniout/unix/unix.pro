include (../../../variables.pri)

TEMPLATE	= lib
LANGUAGE	= C++
TARGET		= peperoniout

INCLUDEPATH	+= . ../../../libs/
CONFIG          += plugin

target.path	= $$OUTPUTPLUGINDIR
INSTALLS	+= target

HEADERS += peperoniout.h peperonidevice.h usbdmx.h
SOURCES += peperoniout.cpp peperonidevice.cpp
