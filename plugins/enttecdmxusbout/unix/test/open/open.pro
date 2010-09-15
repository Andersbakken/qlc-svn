include(../../../../../variables.pri)
include(../../../../../coverage.pri)

TEMPLATE = app
LANGUAGE = C++
TARGET = test_dmxusbopen

QT += core xml gui testlib

INCLUDEPATH += ../../src
unix:!macx:LIBS += -L../../src -lenttecdmxusbout
QMAKE_CXXFLAGS += $$system(pkg-config --cflags libftdi)

HEADERS += enttecdmxusbopen_test.h
SOURCES += enttecdmxusbopen_test.cpp
SOURCES += ../ftdimock/ftdimock.cpp

macx {
	# Link these statically to the test binary since LD_INSERT_LIBRARIES
	# doesn't seem to work with libftdi.
	QTPLUGIN =
	HEADERS += ../../src/enttecdmxusbopen.h
	SOURCES += ../../src/enttecdmxusbopen.cpp
}
