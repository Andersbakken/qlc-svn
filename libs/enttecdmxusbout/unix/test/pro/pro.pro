include(../../../../../variables.pri)
include(../../../../../coverage.pri)

TEMPLATE = app
LANGUAGE = C++
TARGET = test_dmxusbpro

INCLUDEPATH += ../../src

QT += core xml gui testlib
CONFIG += link_pkgconfig
CONFIG -= app_bundle

PKGCONFIG += libftdi
LIBS += -L../../src -lenttecdmxusbout

HEADERS += enttecdmxusbpro_test.h
SOURCES += enttecdmxusbpro_test.cpp
