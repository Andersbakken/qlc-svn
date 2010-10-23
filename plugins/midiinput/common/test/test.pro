include(../../../../variables.pri)

TEMPLATE = app
LANGUAGE = C++
TARGET   = test_common

CONFIG  += qtestlib
QTPLUGIN =

INCLUDEPATH += ../src
DEPENDPATH  += ../src
INCLUDEPATH += ../../../interfaces

HEADERS += midiprotocol_test.h \
           ../src/midiprotocol.h

SOURCES += midiprotocol_test.cpp \
           ../src/midiprotocol.cpp

SOURCES += main.cpp
