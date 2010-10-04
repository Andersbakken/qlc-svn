include (../../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = peperoniout

INCLUDEPATH += peperoni
INCLUDEPATH += ../../interfaces
DEPENDPATH  += peperoni
CONFIG      += plugin

# Headers
HEADERS += peperoni/usbdmx-dynamic.h \
           peperoniout.h \
           peperonidevice.h

# Sources
SOURCES += peperoni/usbdmx-dynamic.cpp \
           peperoniout.cpp \
           peperonidevice.cpp

# Installation
target.path = $$INSTALLROOT/$$OUTPUTPLUGINDIR
INSTALLS   += target
