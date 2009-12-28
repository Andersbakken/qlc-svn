include (../../../variables.pri)

TEMPLATE	= lib
LANGUAGE	= C++
TARGET		= enttecdmxusbproout

CONFIG		+= plugin
INCLUDEPATH	+= ../../../libs/common

# Plugin installation
target.path	= $$OUTPUTPLUGINDIR
INSTALLS	+= target

HEADERS += enttecdmxusbproout.h \
	   enttecdmxusbpro.h

SOURCES += enttecdmxusbproout.cpp \
	   enttecdmxusbpro.cpp

