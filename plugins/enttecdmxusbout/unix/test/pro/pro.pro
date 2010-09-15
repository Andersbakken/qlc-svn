include(../../../../../variables.pri)
include(../../../../../coverage.pri)

TEMPLATE = app
LANGUAGE = C++
TARGET = test_dmxusbpro

QT += core xml gui testlib

INCLUDEPATH += ../../src
unix:!macx:LIBS += -L../../src -lenttecdmxusbout
QMAKE_CXXFLAGS += $$system(pkg-config --cflags libftdi)

HEADERS += enttecdmxusbpro_test.h
SOURCES += enttecdmxusbpro_test.cpp
SOURCES += ../ftdimock/ftdimock.cpp

macx {
	QTPLUGIN =
	CONFIG -= app_bundle
	HEADERS += ../../src/enttecdmxusbpro.h
	SOURCES += ../../src/enttecdmxusbpro.cpp
}
