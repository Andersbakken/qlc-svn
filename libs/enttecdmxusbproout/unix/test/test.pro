include(../../../../variables.pri)

TEMPLATE = app
TARGET = test

CONFIG -= app_bundle
!macx:QT += dbus

INCLUDEPATH += ../
INCLUDEPATH += ../../../common
DEPENDPATH  += ../

SOURCES += main.cpp ../enttecdmxusbpro.cpp ../enttecdmxusbproout.cpp
HEADERS += ../enttecdmxusbpro.h ../enttecdmxusbproout.h
