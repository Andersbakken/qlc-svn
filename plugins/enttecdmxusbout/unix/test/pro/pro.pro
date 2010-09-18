include(../../../../../variables.pri)

TEMPLATE = app
LANGUAGE = C++
TARGET = test_dmxusbpro

QT += core xml gui testlib

INCLUDEPATH += ../../src
LIBS += -L../../src -lenttecdmxusbout
QMAKE_CXXFLAGS += $$system(pkg-config --cflags libftdi)

HEADERS += enttecdmxusbpro_test.h
SOURCES += enttecdmxusbpro_test.cpp
