include(../../../variables.pri)

TEMPLATE 	= lib
LANGUAGE 	= C++
TARGET 		= midiinput

CONFIG          += plugin
INCLUDEPATH	+= ../common
INCLUDEPATH	+= ../../interfaces
DEPENDPATH	+= ../common
LIBS		+= -framework CoreMIDI -framework CoreFoundation

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

# This must be after "TARGET = " and before target installation so that
# install_name_tool can be run before target installation
include(../../../macx/nametool.pri)

target.path	= $$INSTALLROOT/$$INPUTPLUGINDIR
INSTALLS	+= target
