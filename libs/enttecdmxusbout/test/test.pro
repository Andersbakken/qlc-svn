TEMPLATE = app
LANGUAGE = C++
TARGET = test

CONFIG -= release
CONFIG += debug

INCLUDEPATH += ../
INCLUDEPATH += ../ftdi
INCLUDEPATH += ../../common

LIBS += ../ftdi/libftd2xx.a.0.4.16

HEADERS += ../enttecdmxusbout.h \
	   ../enttecdmxusbwidget.h \
	   ../enttecdmxusbpro.h \
	   ../enttecdmxusbopen.h

SOURCES += ../enttecdmxusbout.cpp \
	   ../enttecdmxusbpro.cpp \
	   ../enttecdmxusbopen.cpp \
	   main.cpp
