include (../../variables.pri)

TEMPLATE	= lib
LANGUAGE	= C++
TARGET		= enttecdmxusbout

CONFIG		+= plugin
INCLUDEPATH	+= ../../libs ftdi
macx:LIBS	+= -Lftdi/macx -lftd2xx.0.1.6 -lIOKit
win32:LIBS	+= ftdi/win32/ftd2xx.lib

#ftdi/macx/libftd2xx.a.0.1.6

unix:!macx {
	HARDWARE_PLATFORM = $$system(uname -m)
	contains(HARDWARE_PLATFORM, x86_64) {
		# 64-bit Linux
		LIBS += ftdi/unix/libftd2xx.a.0.4.16_x86-64 -ldl
	} else {
		# 32-bit Linux
		LIBS += ftdi/unix/libftd2xx.a.0.4.16 -ldl
	}
}

# Rules to make ENTTEC devices readable & writable by normal users
unix:!macx:udev.path	= /etc/udev/rules.d
unix:!macx:udev.files	= z65-enttec-dmxusb.rules
unix:!macx:INSTALLS	+= udev

# Plugin installation
target.path	= $$OUTPUTPLUGINDIR
INSTALLS	+= target

# FTDI library installation
macx:QMAKE_POST_LINK 	+= install_name_tool -change /usr/local/lib/libftd2xx.0.1.6.dylib \
			   @executable_path/../Frameworks/FTDI/libftd2xx.0.1.6.dylib \
			   libenttecdmxusbout.dylib
macx:ftdilib.path	= $$LIBSDIR/FTDI
macx:ftdilib.files	= ftdi/macx/libftd2xx.0.1.6.dylib
macx:INSTALLS		+= ftdilib

HEADERS += ftdi/ftd2xx.h \
	   ftdi/WinTypes.h \
	   enttecdmxusbout.h \
	   enttecdmxusbwidget.h \
	   enttecdmxusbpro.h \
	   enttecdmxusbopen.h

SOURCES += enttecdmxusbout.cpp \
	   enttecdmxusbpro.cpp \
	   enttecdmxusbopen.cpp
