include(../../variables.pri)

TEMPLATE	= lib
LANGUAGE	= C++
TARGET		= udmxout

CONFIG          += plugin

INCLUDEPATH	+= . ../../libs/
unix:LIBS	+= -lusb

target.path 	= $$OUTPUTPLUGINDIR
INSTALLS	+= target

# UDEV rule to make uDMX USB device readable & writable for users in Linux
udev.path	= /etc/udev/rules.d
udev.files = z65-anyma-udmx.rules
unix:!macx:INSTALLS	+= udev

# Forms
FORMS += configureudmxout.ui

# Headers
HEADERS += configureudmxout.h \
	   udmxdevice.h \
	   udmxout.h

win32:HEADERS += libusb_dyn.h

# Sources
SOURCES += configureudmxout.cpp \
	   udmxdevice.cpp \
	   udmxout.cpp

win32:SOURCES += libusb_dyn.c

