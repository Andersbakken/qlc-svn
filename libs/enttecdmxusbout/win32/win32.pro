include (../../../variables.pri)

TEMPLATE	= lib
LANGUAGE	= C++
TARGET		= enttecdmxusbout

CONFIG		+= plugin
INCLUDEPATH	+= ../../common
INCLUDEPATH	+= ../ftdi
LIBS		+= ../ftdi/win32/ftd2xx.lib

# Plugin installation
target.path	= $$INSTALLROOT/$$OUTPUTPLUGINDIR
INSTALLS	+= target

HEADERS += ../ftdi/ftd2xx.h \
	   ../ftdi/WinTypes.h \
	   enttecdmxusbout.h \
	   enttecdmxusbwidget.h \
	   enttecdmxusbpro.h \
	   enttecdmxusbopen.h

SOURCES += enttecdmxusbout.cpp \
	   enttecdmxusbpro.cpp \
	   enttecdmxusbopen.cpp
