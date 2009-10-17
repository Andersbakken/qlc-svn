include (../../../variables.pri)

TEMPLATE	= lib
LANGUAGE	= C++
TARGET		= enttecdmxusbout

CONFIG		+= plugin
INCLUDEPATH	+= ../.. ../ftdi
LIBS		+= ../ftdi/win32/ftd2xx.lib

# Plugin installation
target.path	= $$OUTPUTPLUGINDIR
INSTALLS	+= target

HEADERS += ../ftdi/ftd2xx.h \
	   ../ftdi/WinTypes.h \
	   ../unix/enttecdmxusbout.h \
	   ../unix/enttecdmxusbwidget.h \
	   ../unix/enttecdmxusbpro.h \
	   ../unix/enttecdmxusbopen.h

SOURCES += ../unix/enttecdmxusbout.cpp \
	   ../unix/enttecdmxusbpro.cpp \
	   ../unix/enttecdmxusbopen.cpp
