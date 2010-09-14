include(../../variables.pri)

TEMPLATE	= lib
LANGUAGE	= C++
TARGET		= dmx4linuxout

INCLUDEPATH	+= ../interfaces
CONFIG          += plugin

target.path	= $$INSTALLROOT/$$OUTPUTPLUGINDIR
INSTALLS	+= target

HEADERS += dmx4linuxout.h
SOURCES += dmx4linuxout.cpp
