include(../../../variables.pri)

TEMPLATE	= lib
LANGUAGE	= C++
TARGET		= midiout

INCLUDEPATH	+= ../common/ ../../
CONFIG          += plugin
LIBS		+= -lwinmm

target.path	= $$OUTPUTPLUGINDIR
INSTALLS	+= target

FORMS		+= ../common/configuremididevice.ui \
		   ../common/configuremidiout.ui

SOURCES		+= ../common/configuremididevice.cpp \
		   ../common/configuremidiout.cpp \
		   mididevice-win32.cpp \
		   midiout-win32.cpp

HEADERS		+= ../common/configuremididevice.h \
		   ../common/configuremidiout.h \
		   ../common/midiprotocol.h \
		   mididevice-win32.h \
		   midiout-win32.h
