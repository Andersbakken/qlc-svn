include (../../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = enttecdmxusbout

CONFIG      += plugin
QT          += gui core
INCLUDEPATH += ../../interfaces
INCLUDEPATH += ../common

# FTD2XX is a proprietary interface by FTDI Ltd. and would therefore taint the
# 100% FLOSS codebase of QLC if distributed along with QLC sources. Download
# the latest driver package from http://www.ftdichip.com/Drivers/D2XX.htm and
# extract its contents under FTD2XXDIR below to compile this plugin.
#
# Use normal slashes "/" instead of Windows' backslashes "\" for paths !!!!
#
win32 {
    # Windows target
    FTD2XXDIR    = C:/CDM20802
    LIBS        += -L$$FTD2XXDIR/i386 -lftd2xx
    INCLUDEPATH += $$FTD2XXDIR
} else {
    # Unix target for Linux & OSX.
    # OSX: You need to create symlinks for libftd2xx.dylib and libftd2xx.0.dylib pointing
    # to the actual library: sudo ln -s libftd2xx.0.1.7.dylib libftd2xx.dylib
    FTD2XXDIR    = /usr/local
    LIBS        += -L$$FTD2XXDIR/lib -lftd2xx
    INCLUDEPATH += $$FTD2XXDIR/include
}

HEADERS += ../common/enttecdmxusbwidget.h \
           ../common/qlcftdi.h \
           enttecdmxusbout.h \
           enttecdmxusbpro.h \
           enttecdmxusbopen.h

SOURCES += ../common/enttecdmxusbwidget.cpp \
           enttecdmxusbout.cpp \
           enttecdmxusbpro.cpp \
           enttecdmxusbopen.cpp

unix {
    DEFINES     += FTD2XX
    SOURCES += ../common/qlcftdi-ftd2xx.cpp
} else {
    CONFIG    += link_pkgconfig
    PKGCONFIG += libftdi libusb
    SOURCES += ../common/qlcftdi-libftdi.cpp
}

HEADERS += ../../interfaces/qlcoutplugin.h

# This must be after "TARGET = " and before target installation so that
# install_name_tool can be run before target installation
macx:include(../../../macx/nametool.pri)

# Plugin installation
target.path = $$INSTALLROOT/$$OUTPUTPLUGINDIR
INSTALLS   += target
