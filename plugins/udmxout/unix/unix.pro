include(../../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = udmxout

CONFIG      += plugin
INCLUDEPATH += ../../interfaces
CONFIG      += link_pkgconfig
PKGCONFIG   += libusb

HEADERS += udmxdevice.h \
           udmxout.h

SOURCES += udmxdevice.cpp \
           udmxout.cpp

HEADERS += ../../interfaces/qlcoutplugin.h

# This must be after "TARGET = " and before target installation so that
# install_name_tool can be run before target installation
macx:include(../../../macx/nametool.pri)

# Installation
target.path = $$INSTALLROOT/$$OUTPUTPLUGINDIR
INSTALLS   += target

# UDEV rule to make uDMX USB device readable & writable for users in Linux
udev.path  = /etc/udev/rules.d
udev.files = z65-anyma-udmx.rules
INSTALLS  += udev
