include (../../../variables.pri)

TEMPLATE	= lib
LANGUAGE	= C++
TARGET		= enttecdmxusbproout

CONFIG		+= plugin
QT		+= dbus
DEFINES		+= DBUS_ENABLED
INCLUDEPATH	+= ../../common

# Plugin installation
target.path	= $$INSTALLROOT/$$OUTPUTPLUGINDIR
INSTALLS	+= target

HEADERS += enttecdmxusbproout.h \
	   enttecdmxusbpro.h

SOURCES += enttecdmxusbproout.cpp \
	   enttecdmxusbpro.cpp

