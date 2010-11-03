include (../../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = enttecdmxusbout

CONFIG      += plugin
QT          += gui core
INCLUDEPATH += ../../interfaces

# FTD2XX is a proprietary interface by FTDI Ltd. and would therefore taint the
# 100% FLOSS codebase of QLC if distributed along with QLC sources. Download
# the latest driver package from http://www.ftdichip.com/Drivers/D2XX.htm and
# extract its contents under FTD2XXDIR below to compile this plugin.
win32 {
    # Windows target
    FTD2XXDIR    = C:\CDM20802
    LIBS        += -L$$FTD2XXDIR\i386 -lftd2xx
    INCLUDEPATH += $$FTD2XXDIR
} else {
    # Unix target for Linux & OSX
    FTD2XXDIR    = /usr/local
    LIBS        += -L$$FTD2XXDIR/lib -lftd2xx
    INCLUDEPATH += $$FTD2XXDIR/include
}

HEADERS += enttecdmxusbout.h \
           enttecdmxusbwidget.h \
           enttecdmxusbpro.h \
           enttecdmxusbopen.h
       
SOURCES += enttecdmxusbout.cpp \
           enttecdmxusbpro.cpp \
           enttecdmxusbopen.cpp

HEADERS += ../../interfaces/qlcoutplugin.h

# This must be after "TARGET = " and before target installation so that
# install_name_tool can be run before target installation
macx:include(../../../macx/nametool.pri)

# Plugin installation
target.path = $$INSTALLROOT/$$OUTPUTPLUGINDIR
INSTALLS   += target
