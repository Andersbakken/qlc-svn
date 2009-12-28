include (../../../variables.pri)

TEMPLATE	= lib
LANGUAGE	= C++
TARGET		= peperoniout

INCLUDEPATH	+= peperoni
INCLUDEPATH	+= ../../common
DEPENDPATH	+= peperoni
CONFIG          += plugin

target.path	= $$OUTPUTPLUGINDIR
INSTALLS	+= target

HEADERS += peperoni/usbdmx-dynamic.h peperoniout.h peperonidevice.h
SOURCES += peperoni/usbdmx-dynamic.cpp peperoniout.cpp peperonidevice.cpp
