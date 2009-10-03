TEMPLATE = app
LANGUAGE = C++
TARGET = test

INCLUDEPATH += ../ ../ftdi ../../
LIBS += ../ftdi/libftd2xx.a.0.4.16
CONFIG -= release
CONFIG += debug

HEADERS += ../enttecdmxusbout.h \
	   ../enttecdmxusbwidget.h \
	   ../enttecdmxusbpro.h \
	   ../enttecdmxusbopen.h

SOURCES += ../enttecdmxusbout.cpp \
	   ../enttecdmxusbpro.cpp \
	   ../enttecdmxusbopen.cpp \
	   main.cpp
