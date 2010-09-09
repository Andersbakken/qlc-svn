TEMPLATE = app
LANGUAGE = C++
TARGET = test_dmxusbopen

INCLUDEPATH += ../../src

QT += core xml gui testlib
CONFIG += link_pkgconfig
CONFIG -= app_bundle

PKGCONFIG += libftdi

HEADERS += enttecdmxusbopen_test.h ../../src/enttecdmxusbopen.h
SOURCES += enttecdmxusbopen_test.cpp ../../src/enttecdmxusbopen.cpp
