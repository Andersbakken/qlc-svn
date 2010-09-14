include(../../../variables.pri)

TEMPLATE	= lib
LANGUAGE	= C++
TARGET		= midiout

INCLUDEPATH	+= ../common
INCLUDEPATH	+= ../../interfaces
DEPENDPATH	+= ../common
CONFIG          += plugin link_pkgconfig
PKGCONFIG	+= alsa

target.path	= $$INSTALLROOT/$$OUTPUTPLUGINDIR
INSTALLS	+= target

FORMS		+= ../common/configuremididevice.ui \
		   ../common/configuremidiout.ui

HEADERS		+= ../common/configuremididevice.h \
		   ../common/configuremidiout.h \
		   mididevice.h \
		   midiout.h

SOURCES		+= ../common/configuremididevice.cpp \
		   ../common/configuremidiout.cpp \
		   mididevice.cpp \
		   midiout.cpp
