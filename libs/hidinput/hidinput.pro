TEMPLATE 	= lib
LANGUAGE 	= C++
TARGET 		= hidinput

INCLUDEPATH	+= . ../../libs/
CONFIG          += plugin warn_on release

target.path	= /usr/lib/qlc/input
INSTALLS	+= target

# Input
HEADERS += configurehidinput.h \
           hiddevice.h \
           hideventdevice.h \
           hidinput.h \
           hidpoller.h

FORMS += configurehidinput.ui

SOURCES += configurehidinput.cpp \
           hiddevice.cpp \
           hideventdevice.cpp \
           hidinput.cpp \
           hidpoller.cpp
