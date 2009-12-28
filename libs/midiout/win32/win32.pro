include(../../../variables.pri)

TEMPLATE	= lib
LANGUAGE	= C++
TARGET		= midiout

INCLUDEPATH	+= ../common
INCLUDEPATH	+= ../../common
DEPENDPATH	+= ../common
CONFIG          += plugin
LIBS		+= -lwinmm

target.path	= $$OUTPUTPLUGINDIR
INSTALLS	+= target

FORMS		+= ../common/configuremididevice.ui \
		   ../common/configuremidiout.ui

SOURCES		+= ../common/configuremididevice.cpp \
		   ../common/configuremidiout.cpp \
		   mididevice.cpp \
		   midiout.cpp

HEADERS		+= ../common/configuremididevice.h \
		   ../common/configuremidiout.h \
		   ../common/midiprotocol.h \
		   mididevice.h \
		   midiout.h
