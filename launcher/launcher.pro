include(../variables.pri)

TEMPLATE = app
TARGET = qlc-launcher

QT += gui
CONFIG -= app_bundle

RESOURCES += launcher.qrc
INCLUDEPATH += ../libs/common

HEADERS += launcher.h
SOURCES += launcher.cpp main.cpp

# Translations
include(launcher-i18n.pri)

# This must be after "TARGET = " and before target installation so that
# install_name_tool can be run before target installation
include(../macx/nametool.pri)

# Installation
target.path     = $$INSTALLROOT/$$BINDIR
INSTALLS        += target
