TEMPLATE 	= lib
LANGUAGE 	= C++
TARGET 		= midiinput

INCLUDEPATH	+= . ../../libs/
CONFIG          += plugin warn_on debug_and_release build_all

target.path	= /usr/lib/qlc/input
INSTALLS	+= target

# Input
HEADERS += configuremidiinput.h \
           midiinput.h

FORMS += configuremidiinput.ui

SOURCES += configuremidiinput.cpp \
           midiinput.cpp
