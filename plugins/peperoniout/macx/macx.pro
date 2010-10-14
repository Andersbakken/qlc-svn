include(../../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = peperoniout

QT          += core gui
CONFIG      += plugin
INCLUDEPATH += ../../interfaces
CONFIG      += link_pkgconfig
PKGCONFIG   += libusb
DEPENDPATH  += ../unix

HEADERS += ../unix/peperonidevice.h \
           ../unix/peperoniout.h

SOURCES += ../unix/peperonidevice.cpp \
           ../unix/peperoniout.cpp

HEADERS += ../../interfaces/qlcoutplugin.h

# This must be after "TARGET = " and before target installation so that
# install_name_tool can be run before target installation
include(../../../macx/nametool.pri)

target.path = $$INSTALLROOT/$$OUTPUTPLUGINDIR
INSTALLS   += target
