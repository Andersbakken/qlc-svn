include(../../../variables.pri)

TEMPLATE 	= lib
LANGUAGE 	= C++
TARGET 		= midiinput

INCLUDEPATH	+= ../common ../../../libs
CONFIG          += plugin link_pkgconfig
PKGCONFIG	+= alsa

###############################################################################
# Installation
###############################################################################

target.path	= $$INPUTPLUGINDIR
INSTALLS	+= target

###############################################################################
# Sources
###############################################################################

FORMS		+= ../common/configuremidiinput.ui

HEADERS		+= ../common/configuremidiinput.h \
		   ../common/midiinputevent.h \
		   unix-mididevice.h \
		   unix-midiinput.h \
		   unix-midipoller.h

SOURCES 	+= ../common/configuremidiinput.cpp \
		   ../common/midiinputevent.cpp \
		   unix-mididevice.cpp \
		   unix-midiinput.cpp \
		   unix-midipoller.cpp
