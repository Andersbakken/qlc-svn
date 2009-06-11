include(../../variables.pri)

TEMPLATE = app
LANGUAGE = C++
TARGET = test_engine

CONFIG += warn_on qtestlib
QT += xml

INCLUDEPATH += ../ ../../libs/

HEADERS = bus_test.h \
	  ../bus.h

SOURCES = bus_test.cpp \
	  ../bus.cpp \
	  test_engine.cpp
