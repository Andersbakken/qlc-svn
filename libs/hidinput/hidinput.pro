TEMPLATE 	= lib
LANGUAGE 	= C++
TARGET 		= hidinput

INCLUDEPATH	+= . ../../libs/
CONFIG          += plugin warn_on debug_and_release build_all

target.path	= /usr/lib/qlc/input
!macx:INSTALLS	+= target
macx:DESTDIR    = ../../main/qlc.app/Contents/Plugins/input

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
