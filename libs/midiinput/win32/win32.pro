include(../../../variables.pri)

TEMPLATE 	= lib
LANGUAGE 	= C++
TARGET 		= midiinput

CONFIG          += plugin
INCLUDEPATH	+= ../common ../../../libs
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
		   win32-mididevice.h \
		   win32-midiinput.h

SOURCES		+= ../common/configuremidiinput.cpp \
		   ../common/configuremidiline.cpp \
		   ../common/midiinputevent.cpp \
		   win32-mididevice.cpp \
		   win32-midiinput.cpp
