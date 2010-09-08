TEMPLATE = app
LANGUAGE = C++
TARGET = test_enttec

QT += core xml gui testlib
CONFIG += link_pkgconfig
CONFIG -= app_bundle

PKGCONFIG += libftdi

HEADERS += enttecdmxusbopen_test.h \
	   ../enttecdmxusbopen.h
SOURCES += enttecdmxusbopen_test.cpp \
	   ../enttecdmxusbopen.cpp \
	   main.cpp
