include(../../variables.pri)

TEMPLATE = lib
LANGUAGE = C++
TARGET   = olaout

QT       += core gui
CONFIG   += plugin
QTPLUGIN  =

INCLUDEPATH += ../interfaces
LIBS        += -L/usr/local/lib -lolaserver -lola -lolacommon -lprotobuf

# Forms
FORMS += configureolaout.ui

# Headers
HEADERS += olaout.h \
           olaoutthread.h \
           configureolaout.h \
           qlclogdestination.h

# Source
SOURCES += olaout.cpp \
           olaoutthread.cpp \
           configureolaout.cpp \
           qlclogdestination.cpp

HEADERS += ../interfaces/qlcoutplugin.h

PRO_FILE = olaout.pro
TRANSLATIONS += OLA_Output_fi_FI.ts
TRANSLATIONS += OLA_Output_de_DE.ts
include(../../i18n.pri)

# This must be after "TARGET = " and before target installation so that
# install_name_tool can be run before target installation
macx {
    include(../../macx/nametool.pri)
}

# Installation
target.path = $$INSTALLROOT/$$OUTPUTPLUGINDIR
INSTALLS   += target
