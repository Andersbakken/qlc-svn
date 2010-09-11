TEMPLATE = app
LANGUAGE = C++
TARGET = test_dmxusbpro

INCLUDEPATH += ../../src

QT += core xml gui testlib
CONFIG += link_pkgconfig
CONFIG -= app_bundle

PKGCONFIG += libftdi

HEADERS += enttecdmxusbpro_test.h ../../src/enttecdmxusbpro.h
SOURCES += enttecdmxusbpro_test.cpp ../../src/enttecdmxusbpro.cpp
