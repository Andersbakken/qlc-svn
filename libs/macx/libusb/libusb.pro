include(../../../variables.pri)

TEMPLATE = lib
TARGET = usb
VERSION = 0.1.12

CONFIG += staticlib
LIBS += -lIOKit

# Input
HEADERS += error.h usb.h usbi.h
SOURCES += darwin.c descriptors.c error.c usb.c
