include (../../../variables.pri)

TEMPLATE	= lib
LANGUAGE	= C++
TARGET		= enttecdmxusbout

CONFIG		+= plugin
QT		+= dbus
DEFINES		+= DBUS_ENABLED

INCLUDEPATH	+= ../../common
INCLUDEPATH	+= ../ftdi

HARDWARE_PLATFORM = $$system(uname -m)
contains(HARDWARE_PLATFORM, x86_64) {
	# 64-bit Linux
	LIBS += ../ftdi/unix/libftd2xx.a.0.4.16_x86-64 -ldl
} else {
	# 32-bit Linux
	LIBS += ../ftdi/unix/libftd2xx.a.0.4.16 -ldl
}

# Rules to make ENTTEC devices readable & writable by normal users
udev.path	= /etc/udev/rules.d
udev.files	= z65-enttec-dmxusb.rules
INSTALLS	+= udev

# Plugin installation
target.path	= $$OUTPUTPLUGINDIR
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
