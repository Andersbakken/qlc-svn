include(../../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = udmxout

CONFIG      += plugin
INCLUDEPATH += ../../interfaces
INCLUDEPATH += . # For libusb_dyn.*
DEPENDPATH  += ../unix

target.path = $$INSTALLROOT/$$OUTPUTPLUGINDIR
INSTALLS   += target

# Headers
HEADERS += ../unix/udmxdevice.h \
           ../unix/udmxout.h \
           libusb_dyn.h

# Sources
SOURCES += ../unix/udmxdevice.cpp \
           ../unix/udmxout.cpp \
           libusb_dyn.c

# Interfaces
HEADERS += ../../interfaces/qlcoutplugin.h
