include (../../variables.pri)

TEMPLATE	= lib
LANGUAGE	= C++
TARGET		= enttecdmxusbout

INCLUDEPATH	+= ../../libs ftdi
LIBS 		+= ftdi/libftd2xx.a.0.4.16
CONFIG		+= plugin

# Rules to make ENTTEC devices readable & writable by normal users
unix:!macx:udev.path	= /etc/udev/rules.d
unix:!macx:udev.files	= z65-enttec.rules

target.path	= $$OUTPUTPLUGINDIR
!macx:INSTALLS	+= target udev

macx:DESTDIR	= ../../main/qlc.app/Contents/Plugins/output

HEADERS += ftdi/ftd2xx.h \
	   ftdi/WinTypes.h \
	   enttecdmxusbout.h \
	   enttecdmxusbwidget.h \
	   enttecdmxusbpro.h

SOURCES += enttecdmxusbout.cpp \
	   enttecdmxusbpro.cpp
