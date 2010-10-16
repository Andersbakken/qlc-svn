include(../../variables.pri)

TEMPLATE = app
LANGUAGE = C++
TARGET   = qlc

CONFIG += qt
QT     += core xml gui

# Engine
INCLUDEPATH += ../../engine/src
DEPENDPATH  += ../../engine/src
unix:LIBS   += -L../../engine/src -lqlcengine
win32:{
    CONFIG(release, debug|release) LIBS += -L../../engine/src/release -lqlcengine
    CONFIG(debug, debug|release) LIBS += -L../../engine/src/debug -lqlcengine
}

# Types
INCLUDEPATH += ../../plugins/interfaces

# Resources
RESOURCES    += main.qrc
win32:RC_FILE = main.rc

# Sources
HEADERS += aboutbox.h \
           addfixture.h \
           addvcbuttonmatrix.h \
           app.h \
           assignhotkey.h \
           busmanager.h \
           chasereditor.h \
           collectioneditor.h \
           consolechannel.h \
           docbrowser.h \
           efxeditor.h \
           fixtureconsole.h \
           fixturemanager.h \
           fixtureselection.h \
           functionmanager.h \
           functionselection.h \
           functionwizard.h \
           generatorarea.h \
           inputchanneleditor.h \
           inputprofileeditor.h \
           inputmanager.h \
           inputpatcheditor.h \
           monitor.h \
           monitorfixture.h \
           monitorlayout.h \
           outputmanager.h \
           outputpatcheditor.h \
           sceneeditor.h \
           selectinputchannel.h \
           vcbutton.h \
           vcbuttonproperties.h \
           vccuelist.h \
           vccuelistproperties.h \
           vcdockarea.h \
           vcdockslider.h \
           vcframe.h \
           vclabel.h \
           vcproperties.h \
           vcslider.h \
           vcsliderproperties.h \
           vcsoloframe.h \
           vcwidget.h \
           vcwidgetproperties.h \
           vcxypad.h \
           vcxypadfixture.h \
           vcxypadfixtureeditor.h \
           vcxypadproperties.h \
           virtualconsole.h

FORMS += aboutbox.ui \
         addfixture.ui \
         addvcbuttonmatrix.ui \
         assignhotkey.ui \
         chasereditor.ui \
         collectioneditor.ui \
         efxeditor.ui \
         fixtureselection.ui \
         functionselection.ui \
         functionwizard.ui \
         inputchanneleditor.ui \
         inputprofileeditor.ui \
         inputpatcheditor.ui \
         outputpatcheditor.ui \
         sceneeditor.ui \
         selectinputchannel.ui \
         vcbuttonproperties.ui \
         vccuelistproperties.ui \
         vcdockslider.ui \
         vcproperties.ui \
         vcsliderproperties.ui \
         vcxypadfixtureeditor.ui \
         vcxypadproperties.ui

SOURCES += aboutbox.cpp \
           addfixture.cpp \
           addvcbuttonmatrix.cpp \
           app.cpp \
           assignhotkey.cpp \
           busmanager.cpp \
           chasereditor.cpp \
           collectioneditor.cpp \
           consolechannel.cpp \
           docbrowser.cpp \
           efxeditor.cpp \
           fixtureconsole.cpp \
           fixturemanager.cpp \
           fixtureselection.cpp \
           functionmanager.cpp \
           functionselection.cpp \
           functionwizard.cpp \
           generatorarea.cpp \
           inputchanneleditor.cpp \
           inputprofileeditor.cpp \
           inputmanager.cpp \
           inputpatcheditor.cpp \
           main.cpp \
           monitor.cpp \
           monitorfixture.cpp \
           monitorlayout.cpp \
           outputmanager.cpp \
           outputpatcheditor.cpp \
           sceneeditor.cpp \
           selectinputchannel.cpp \
           vcbutton.cpp \
           vcbuttonproperties.cpp \
           vccuelist.cpp \
           vccuelistproperties.cpp \
           vcdockarea.cpp \
           vcdockslider.cpp \
           vcframe.cpp \
           vclabel.cpp \
           vcproperties.cpp \
           vcslider.cpp \
           vcsliderproperties.cpp \
           vcsoloframe.cpp \
           vcwidget.cpp \
           vcwidgetproperties.cpp \
           vcxypad.cpp \
           vcxypadfixture.cpp \
           vcxypadfixtureeditor.cpp \
           vcxypadproperties.cpp \
           virtualconsole.cpp

# Internationalization
PRO_FILE      = src.pro
TRANSLATIONS += qlc_fi_FI.ts
TRANSLATIONS += qlc_fr_FR.ts
TRANSLATIONS += qlc_es_ES.ts
TRANSLATIONS += qlc_de_DE.ts
include(../../i18n.pri)

macx {
    # This must be after "TARGET = " and before target installation so that
    # install_name_tool can be run before target installation
    include(../../macx/nametool.pri)
}

# Installation
target.path = $$INSTALLROOT/$$BINDIR
INSTALLS   += target
