include (../../../variables.pri)

TEMPLATE	= lib
LANGUAGE	= C++
TARGET		= enttecdmxusbproout

CONFIG		+= plugin
unix:!macx:QT	+= dbus
INCLUDEPATH	+= ../../../libs

# Plugin installation
target.path	= $$OUTPUTPLUGINDIR
macx:INSTALLS	+= target

HEADERS += enttecdmxusbproout.h \
	   enttecdmxusbpro.h

SOURCES += enttecdmxusbproout.cpp \
	   enttecdmxusbpro.cpp

