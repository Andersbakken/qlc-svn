include (../../../variables.pri)

TEMPLATE	= lib
LANGUAGE	= C++
TARGET		= enttecdmxusbproout

CONFIG		+= plugin
QT		+= dbus
INCLUDEPATH	+= ../../../libs

# Plugin installation
target.path	= $$OUTPUTPLUGINDIR
!macx:INSTALLS	+= target
macx:DESTDIR	= ../../../main/qlc.app/Contents/Plugins/output

HEADERS += enttecdmxusbproout.h \
	   enttecdmxusbpro.h

SOURCES += enttecdmxusbproout.cpp \
	   enttecdmxusbpro.cpp

