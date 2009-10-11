include (../../variables.pri)

TEMPLATE	= lib
LANGUAGE	= C++
TARGET		= enttecdmxusbout

CONFIG		+= plugin
INCLUDEPATH	+= ../../libs ftdi
macx:LIBS	+= ftdi/macx/libftd2xx.a.0.1.6 -lIOKit
win32:LIBS	+= ftdi/win32/ftd2xx.lib

unix:!macx {
	HARDWARE_PLATFORM = $$system(uname -m)
	contains(HARDWARE_PLATFORM, x86_64) {
		# 64-bit Linux
		LIBS += ftdi/libftd2xx.a.0.4.16_x86-64 -ldl
	} else {
		# 32-bit Linux
		LIBS += ftdi/libftd2xx.a.0.4.16 -ldl
	}
}

# Rules to make ENTTEC devices readable & writable by normal users
unix:!macx:udev.path	= /etc/udev/rules.d
unix:!macx:udev.files	= z65-enttec-dmxusb.rules
unix:!macx:INSTALLS	+= udev

# Plugin installation
target.path	= $$OUTPUTPLUGINDIR
INSTALLS	+= target

HEADERS += ftdi/ftd2xx.h \
	   ftdi/WinTypes.h \
	   enttecdmxusbout.h \
	   enttecdmxusbwidget.h \
	   enttecdmxusbpro.h \
	   enttecdmxusbopen.h

SOURCES += enttecdmxusbout.cpp \
	   enttecdmxusbpro.cpp \
	   enttecdmxusbopen.cpp

#macx {
#	QMAKE_POST_LINK = cp ftdi/libftd2xx.0.1.4.dylib ../../main/qlc.app/Contents/Plugins/output/
#	QMAKE_POST_LINK += && install_name_tool -change /usr/local/lib/libftd2xx.0.1.4.dylib @executable_path/../Plugins/output/libftd2xx.0.1.4.dylib ../../main/qlc.app/Contents/Plugins/output/libenttecdmxusbout.dylib
#}
