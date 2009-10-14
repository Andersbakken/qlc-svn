include (../../../variables.pri)

TEMPLATE	= lib
LANGUAGE	= C++
TARGET		= peperoniout

INCLUDEPATH	+= . ../../../libs/
CONFIG          += plugin

target.path	= $$OUTPUTPLUGINDIR
INSTALLS	+= target

# UDEV rule to make Peperoni USB devices readable & writable for users in Linux
udev.path   = /etc/udev/rules.d
udev.files  = z65-peperoni.rules
INSTALLS    += udev

HEADERS += peperoniout.h peperonidevice.h usbdmx.h
SOURCES += peperoniout.cpp peperonidevice.cpp
