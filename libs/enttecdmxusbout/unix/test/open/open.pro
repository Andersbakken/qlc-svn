include(../../../../../variables.pri)
include(../../../../../coverage.pri)

TEMPLATE = app
LANGUAGE = C++
TARGET = test_dmxusbopen

INCLUDEPATH += ../../src

QT += core xml gui testlib
CONFIG += link_pkgconfig
CONFIG -= app_bundle

PKGCONFIG += libftdi
LIBS += -L../../src -lenttecdmxusbout

HEADERS += enttecdmxusbopen_test.h
SOURCES += enttecdmxusbopen_test.cpp
