include(../../variables.pri)

TEMPLATE	= lib
LANGUAGE	= C++
TARGET		= udmxout

CONFIG          += plugin warn_on release

INCLUDEPATH		+= . ../../libs/

win32:INCLUDEPATH	+= C:/Projekte/Libs/libusb-win32\src
win32:LIBS 		+= -LC:/Projekte/Libs/libusb-win32 -lusb

target.path 	= $$OUTPUTPLUGINDIR
INSTALLS	+= target

# Forms
FORMS += configureudmxout.ui

# Headers
HEADERS += configureudmxout.h \
	   udmxout.h

# Sources
SOURCES += configureudmxout.cpp \
	   udmxout.cpp
