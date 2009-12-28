TEMPLATE = app
TARGET = test

CONFIG += warn_on
macx:CONFIG -= app_bundle
!macx:QT += dbus

INCLUDEPATH += ../
INCLUDEPATH += ../../../common
DEPENDPATH  += ../

SOURCES += main.cpp ../enttecdmxusbpro.cpp ../enttecdmxusbproout.cpp
HEADERS += ../enttecdmxusbpro.h ../enttecdmxusbproout.h
