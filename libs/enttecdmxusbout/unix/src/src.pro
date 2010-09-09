include (../../../../variables.pri)

TEMPLATE	= lib
LANGUAGE	= C++
TARGET		= enttecdmxusbout

INCLUDEPATH	+= ../../../common

CONFIG		+= plugin
CONFIG		+= link_pkgconfig
PKGCONFIG	+= libftdi
macx:PKGCONFIG  += libusb

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

PRO_FILE = src.pro
TRANSLATIONS += enttec_fi_FI.ts
include(../../../../i18n.pri)

# This must be after "TARGET = " and before target installation so that
# install_name_tool can be run before target installation
macx:include(../../../../macx/nametool.pri)

target.path	= $$INSTALLROOT/$$OUTPUTPLUGINDIR
INSTALLS	+= target
