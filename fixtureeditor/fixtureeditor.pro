include(../variables.pri)

TEMPLATE = app
LANGUAGE = C++
TARGET   = qlc-fixtureeditor

CONFIG += qt
QT     += xml

INCLUDEPATH += ../plugins/interfaces

INCLUDEPATH += ../engine/src
DEPENDPATH  += ../engine/src

INCLUDEPATH += ../ui/src
DEPENDPATH  += ../ui/src
QMAKE_LIBDIR += ../engine/src 
LIBS    += -lqlcengine

# Sources
RESOURCES    += ../ui/src/main.qrc
win32:RC_FILE = fixtureeditor.rc

HEADERS += ../ui/src/aboutbox.h \
           ../ui/src/docbrowser.h \
           ../ui/src/apputil.h \
           app.h \
           capabilitywizard.h \
           editcapability.h \
           editchannel.h \
           editmode.h \
           fixtureeditor.h

FORMS += ../ui/src/aboutbox.ui \
         capabilitywizard.ui \
         editcapability.ui \
         editchannel.ui \
         editmode.ui \
         fixtureeditor.ui

SOURCES += ../ui/src/aboutbox.cpp \
           ../ui/src/docbrowser.cpp \
           ../ui/src/apputil.cpp \
           app.cpp \
           capabilitywizard.cpp \
           editcapability.cpp \
           editchannel.cpp \
           editmode.cpp \
           fixtureeditor.cpp \
           main.cpp

# Internationalization
PRO_FILE      = fixtureeditor.pro
TRANSLATIONS += fixtureeditor_fi_FI.ts
TRANSLATIONS += fixtureeditor_fr_FR.ts
TRANSLATIONS += fixtureeditor_de_DE.ts
TRANSLATIONS += fixtureeditor_es_ES.ts
include(../i18n.pri)

macx {
    # This must be after "TARGET = " and before target installation so that
    # install_name_tool can be run before target installation
    include(../macx/nametool.pri)
}

# Installation
target.path = $$INSTALLROOT/$$BINDIR
INSTALLS   += target
