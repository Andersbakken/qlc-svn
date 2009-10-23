include(../../../variables.pri)

TEMPLATE 	= lib
LANGUAGE 	= C++
TARGET 		= midiinput

CONFIG          += plugin
INCLUDEPATH	+= ../common ../..
LIBS		+= -lwinmm

###############################################################################
# Installation
###############################################################################

target.path	= $$INPUTPLUGINDIR
INSTALLS	+= target

###############################################################################
# Sources
###############################################################################

FORMS		+= ../common/configuremidiinput.ui \
		   ../common/configuremidiline.ui

HEADERS		+= ../common/configuremidiinput.h \
		   ../common/configuremidiline.h \
   		   ../common/midiinputevent.h \
		   mididevice.h \
		   midiinput.h

SOURCES		+= ../common/configuremidiinput.cpp \
		   ../common/configuremidiline.cpp \
		   ../common/midiinputevent.cpp \
		   mididevice.cpp \
		   midiinput.cpp
