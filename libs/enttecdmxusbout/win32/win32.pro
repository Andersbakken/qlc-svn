include (../../../variables.pri)

TEMPLATE	= lib
LANGUAGE	= C++
TARGET		= enttecdmxusbout

CONFIG		+= plugin
INCLUDEPATH	+= ../../common
LIBS		+= ftd2xx.lib

# Plugin installation
target.path	= $$INSTALLROOT/$$OUTPUTPLUGINDIR
INSTALLS	+= target

HEADERS += enttecdmxusbout.h \
	   enttecdmxusbwidget.h \
	   enttecdmxusbpro.h \
	   enttecdmxusbopen.h \
	   ftd2xx.h
	   
SOURCES += enttecdmxusbout.cpp \
	   enttecdmxusbpro.cpp \
	   enttecdmxusbopen.cpp
