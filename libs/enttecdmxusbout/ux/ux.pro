include (../../../variables.pri)

TEMPLATE	= lib
LANGUAGE	= C++
TARGET		= enttecdmxusbout

CONFIG		+= plugin

unix:!macx {
	QT	+= dbus
	DEFINES += DBUS_ENABLED

	# Rules to make ENTTEC devices readable & writable by normal users
	udev.path	= /etc/udev/rules.d
	udev.files	= ../unix/z65-enttec-dmxusb.rules
	INSTALLS	+= udev
}

INCLUDEPATH	+= ../../common

CONFIG += link_pkgconfig
PKGCONFIG += libftdi

# Plugin installation
target.path	= $$INSTALLROOT/$$OUTPUTPLUGINDIR
INSTALLS	+= target

HEADERS += ../unix/enttecdmxusbwidget.h \
	   enttecdmxusbout.h \
	   enttecdmxusbpro.h \
	   enttecdmxusbopen.h

SOURCES += enttecdmxusbout.cpp \
	   enttecdmxusbopen.cpp \
	   enttecdmxusbpro.cpp
