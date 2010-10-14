include(../../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = peperoniout

CONFIG      += plugin
INCLUDEPATH += ../../interfaces
LIBS        += -lusb

target.path = $$INSTALLROOT/$$OUTPUTPLUGINDIR
INSTALLS   += target

# UDEV rule to make Peperoni USB devices readable & writable for users in Linux
udev.path  = /etc/udev/rules.d
udev.files = z65-peperoni.rules
INSTALLS  += udev

HEADERS += peperonidevice.h \
           peperoniout.h

SOURCES += peperonidevice.cpp \
           peperoniout.cpp

HEADERS += ../../interfaces/qlcoutplugin.h
