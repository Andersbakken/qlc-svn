include(../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = vellemanout

QT -= gui

INCLUDEPATH += ../interfaces
CONFIG      += plugin
LIBS        += K8062D.lib

HEADERS += vellemanout.h
SOURCES += vellemanout.cpp
HEADERS += ../interfaces/qlcoutplugin.h

target.path = $$INSTALLROOT/$$OUTPUTPLUGINDIR
INSTALLS   += target
