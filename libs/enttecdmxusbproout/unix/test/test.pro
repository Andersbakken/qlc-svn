TEMPLATE = app
TARGET = test

CONFIG += warn_on
QT += dbus

INCLUDEPATH += .. ../../../

SOURCES += main.cpp ../enttecdmxusbpro.cpp ../enttecdmxusbproout.cpp
HEADERS += ../enttecdmxusbpro.h ../enttecdmxusbproout.h
