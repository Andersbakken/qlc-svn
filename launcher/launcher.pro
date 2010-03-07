include(../variables.pri)

TEMPLATE = app
TARGET = qlc-launcher

QT += gui
CONFIG -= app_bundle

RESOURCES += launcher.qrc
INCLUDEPATH += ../libs/common

HEADERS += launcher.h
SOURCES += launcher.cpp main.cpp

# Installation
target.path     = $$INSTALLROOT/$$BINDIR
INSTALLS        += target
