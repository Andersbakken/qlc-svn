include (../../../variables.pri)

TEMPLATE	= lib
LANGUAGE	= C++
TARGET		= enttecdmxusbproout

CONFIG		+= plugin
INCLUDEPATH	+= ../../common
INCLUDEPATH	+= ../unix
DEPENDPATH	+= ../unix

# Plugin installation
target.path	= $$INSTALLROOT/$$OUTPUTPLUGINDIR
INSTALLS	+= target

HEADERS += ../unix/enttecdmxusbproout.h \
	   ../unix/enttecdmxusbpro.h

SOURCES += ../unix/enttecdmxusbproout.cpp \
	   ../unix/enttecdmxusbpro.cpp

