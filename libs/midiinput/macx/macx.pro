include(../../../variables.pri)

TEMPLATE 	= lib
LANGUAGE 	= C++
TARGET 		= midiinput

CONFIG          += plugin
INCLUDEPATH	+= ../common ../..
LIBS		+= -framework CoreMIDI

###############################################################################
# Installation
###############################################################################

target.path	= $$INPUTPLUGINDIR
INSTALLS	+= target

###############################################################################
# Sources
###############################################################################

HEADERS		+= ../common/midiinputevent.h \
		   mididevice.h \
		   midiinput.h

SOURCES		+= ../common/midiinputevent.cpp \
		   mididevice.cpp \
		   midiinput.cpp
