include (../../../variables.pri)

TEMPLATE	= lib
LANGUAGE	= C++
TARGET		= enttecdmxusbout

CONFIG		+= plugin
CONFIG		+= link_pkgconfig
PKGCONFIG	+= libftdi

INCLUDEPATH	+= ../../common

HEADERS += enttecdmxusbwidget.h \
	   enttecdmxusbout.h \
	   enttecdmxusbpro.h \
	   enttecdmxusbopen.h

SOURCES += enttecdmxusbout.cpp \
	   enttecdmxusbopen.cpp \
	   enttecdmxusbpro.cpp

unix:!macx {
	QT	+= dbus
	DEFINES += DBUS_ENABLED

	# Rules to make ENTTEC devices readable & writable by normal users
	udev.path	= /etc/udev/rules.d
	udev.files	= ../unix/z65-enttec-dmxusb.rules
	INSTALLS	+= udev
}

target.path	= $$INSTALLROOT/$$OUTPUTPLUGINDIR
INSTALLS	+= target
