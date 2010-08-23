include (../../../variables.pri)

TEMPLATE	= lib
LANGUAGE	= C++
TARGET		= enttecdmxusbout

CONFIG		+= plugin
INCLUDEPATH	+= ../../common

# FTD2XX is a proprietary interface by FTDI Ltd. and would therefore taint the
# 100% FLOSS codebase of QLC if distributed along with QLC sources. Download
# the latest driver package from http://www.ftdichip.com/Drivers/D2XX.htm and
# extract its contents under FTD2XXDIR below to compile this plugin.
FTD2XXDIR	 = C:\CDM20802
LIBS		+= -L$$FTD2XXDIR\i386 -lftd2xx
INCLUDEPATH	+= $$FTD2XXDIR

# Plugin installation
target.path	= $$INSTALLROOT/$$OUTPUTPLUGINDIR
INSTALLS	+= target

HEADERS += enttecdmxusbout.h \
	   enttecdmxusbwidget.h \
	   enttecdmxusbpro.h \
	   enttecdmxusbopen.h
	   
SOURCES += enttecdmxusbout.cpp \
	   enttecdmxusbpro.cpp \
	   enttecdmxusbopen.cpp
