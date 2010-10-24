include(../../../../variables.pri)
include(../../../../coverage.pri)

CONFIG(coverage) {
    TEMPLATE = lib
    LANGUAGE = C++
    TARGET   = midiprotocol

    INCLUDEPATH += ../../../interfaces

    HEADERS += midiprotocol.h
    SOURCES += midiprotocol.cpp
}
