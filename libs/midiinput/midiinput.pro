TEMPLATE 	= lib
LANGUAGE 	= C++
TARGET 		= midiinput

INCLUDEPATH	+= . ../../libs/
CONFIG          += plugin warn_on debug_and_release build_all

target.path	= /usr/lib/qlc/input
INSTALLS	+= target

# Input
HEADERS += configuremidiinput.h \
	   mididevice.h \
           midiinput.h \
	   midipoller.h

FORMS += configuremidiinput.ui

unix:SOURCES += configuremidiinput.cpp \
		unix-mididevice.cpp \
		unix-midipoller.cpp \
		unix-midiinput.cpp
