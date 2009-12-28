include (../../../variables.pri)

TEMPLATE	= lib
LANGUAGE	= C++
TARGET		= enttecdmxusbout

CONFIG		+= plugin

INCLUDEPATH	+= ../../common
INCLUDEPATH	+= ../ftdi

LIBS		+= -L../ftdi/macx -lftd2xx.0.1.6
LIBS		+= -framework IOKit -framework CoreFoundation

# Plugin installation
target.path	= $$OUTPUTPLUGINDIR
INSTALLS	+= target

# FTDI library installation
QMAKE_POST_LINK += install_name_tool -change /usr/local/lib/libftd2xx.0.1.6.dylib \
		   @executable_path/../Frameworks/FTDI/libftd2xx.0.1.6.dylib \
		   libenttecdmxusbout.dylib
ftdilib.path	= $$LIBSDIR/FTDI
ftdilib.files	= ../ftdi/macx/libftd2xx.0.1.6.dylib
INSTALLS	+= ftdilib

HEADERS += ../ftdi/ftd2xx.h \
	   ../ftdi/WinTypes.h \
	   ../unix/enttecdmxusbout.h \
	   ../unix/enttecdmxusbwidget.h \
	   ../unix/enttecdmxusbpro.h \
	   ../unix/enttecdmxusbopen.h

SOURCES += ../unix/enttecdmxusbout.cpp \
	   ../unix/enttecdmxusbpro.cpp \
	   ../unix/enttecdmxusbopen.cpp
