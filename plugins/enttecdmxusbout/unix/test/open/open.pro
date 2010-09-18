include(../../../../../variables.pri)

TEMPLATE = app
LANGUAGE = C++
TARGET = test_dmxusbopen

QT += core xml gui testlib

INCLUDEPATH += ../../src
LIBS += -L../../src -lenttecdmxusbout
QMAKE_CXXFLAGS += $$system(pkg-config --cflags libftdi)

HEADERS += enttecdmxusbopen_test.h
SOURCES += enttecdmxusbopen_test.cpp
