include(../../../variables.pri)

TEMPLATE    = lib
LANGUAGE    = C++
TARGET	    = enttecdmxusbout

CONFIG	    += plugin

INCLUDEPATH += ../../common

CONFIG      += link_pkgconfig
PKGCONFIG   += libftdi libusb

HEADERS     += ../unix/enttecdmxusbout.h \
               ../unix/enttecdmxusbwidget.h \
               ../unix/enttecdmxusbpro.h \
               ../unix/enttecdmxusbopen.h

SOURCES     += ../unix/enttecdmxusbout.cpp \
               ../unix/enttecdmxusbpro.cpp \
               ../unix/enttecdmxusbopen.cpp

# This must be after "TARGET = " and before target installation so that
# install_name_tool can be run before target installation
include(../../../macx/nametool.pri)

target.path	 = $$INSTALLROOT/$$OUTPUTPLUGINDIR
INSTALLS	+= target
