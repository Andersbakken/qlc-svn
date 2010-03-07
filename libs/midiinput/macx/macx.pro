include(../../../variables.pri)

TEMPLATE 	= lib
LANGUAGE 	= C++
TARGET 		= midiinput

CONFIG          += plugin
INCLUDEPATH	+= ../common
INCLUDEPATH	+= ../../common
DEPENDPATH	+= ../common
LIBS		+= -framework CoreMIDI -framework CoreFoundation

###############################################################################
# Installation
###############################################################################

target.path	= $$INSTALLROOT/$$INPUTPLUGINDIR
INSTALLS	+= target

###############################################################################
# Sources
###############################################################################

FORMS		+= ../common/configuremidiinput.ui \
		   ../common/configuremidiline.ui

HEADERS		+= ../common/midiinputevent.h \
		   ../common/configuremidiinput.h \
		   ../common/configuremidiline.h \
		   mididevice.h \
		   midiinput.h

SOURCES		+= ../common/midiinputevent.cpp \
		   ../common/configuremidiinput.cpp \
		   ../common/configuremidiline.cpp \
		   mididevice.cpp \
		   midiinput.cpp
