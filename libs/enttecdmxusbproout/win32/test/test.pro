include(../../../../variables.pri)

TEMPLATE = app
LANGUAGE = C++
TARGET = test

CONFIG += console

INCLUDEPATH += ..
INCLUDEPATH += ../../../common
DEPENDPATH  += ..

SOURCES += main.cpp ../enttecdmxusbpro.cpp ../enttecdmxusbproout.cpp
HEADERS += ../enttecdmxusbpro.h ../enttecdmxusbproout.h
