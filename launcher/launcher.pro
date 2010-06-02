include(../variables.pri)

TEMPLATE = app
TARGET = qlc-launcher

QT += gui
CONFIG -= app_bundle

RESOURCES += launcher.qrc
INCLUDEPATH += ../libs/common

HEADERS += launcher.h
SOURCES += launcher.cpp main.cpp

# Translation source files
TRANSLATIONS	+= launcher_fi_FI.ts

# Translation object files
translations.path   = $$INSTALLROOT/$$TRANSLATIONDIR
translations.files += launcher_fi_FI.qm
INSTALLS 	   += translations

# Just a hack to force qmake to create all .qm files (along with _fi_FI.qm)
i18n.target = launcher_fi_FI.qm
i18n.commands += lrelease launcher.pro
QMAKE_EXTRA_TARGETS += i18n
PRE_TARGETDEPS += launcher_fi_FI.qm
QMAKE_CLEAN += launcher_fi_FI.qm
QMAKE_DISTCLEAN += launcher_fi_FI.qm

# This must be after "TARGET = " and before target installation so that
# install_name_tool can be run before target installation
include(../macx/nametool.pri)

# Installation
target.path     = $$INSTALLROOT/$$BINDIR
INSTALLS        += target
