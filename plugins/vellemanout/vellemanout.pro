include(../../variables.pri)

TEMPLATE     = lib
LANGUAGE     = C++
TARGET       = vellemanout

INCLUDEPATH += ../interfaces
CONFIG      += plugin
LIBS        += K8062D.lib

HEADERS     += vellemanout.h
SOURCES     += vellemanout.cpp

target.path  = $$INSTALLROOT/$$OUTPUTPLUGINDIR
INSTALLS    += target
