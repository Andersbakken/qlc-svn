include(../variables.pri)

TEMPLATE = app
TARGET = qlc-launcher

QT += core gui

RESOURCES += launcher.qrc

INCLUDEPATH += ../plugins/interfaces
INCLUDEPATH += ../engine/src

HEADERS += launcher.h
SOURCES += launcher.cpp main.cpp

# Translations
# Internationalization
PRO_FILE      = launcher.pro
TRANSLATIONS += launcher_fi_FI.ts
TRANSLATIONS += launcher_de_DE.ts
include(../i18n.pri)

# This must be after "TARGET = " and before target installation so that
# install_name_tool can be run before target installation
include(../macx/nametool.pri)

# Installation
target.path     = $$INSTALLROOT/$$BINDIR
INSTALLS        += target
