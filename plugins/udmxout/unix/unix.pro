include(../../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = udmxout

CONFIG      += plugin
INCLUDEPATH += ../../interfaces
LIBS        += -lusb

# UDEV rule to make uDMX USB device readable & writable for users in Linux
udev.path  = /etc/udev/rules.d
udev.files = z65-anyma-udmx.rules
INSTALLS  += udev

# Headers
HEADERS += udmxdevice.h \
           udmxout.h

# Sources
SOURCES += udmxdevice.cpp \
           udmxout.cpp

HEADERS += ../../interfaces/qlcoutplugin.h

# Installation
target.path = $$INSTALLROOT/$$OUTPUTPLUGINDIR
INSTALLS   += target
