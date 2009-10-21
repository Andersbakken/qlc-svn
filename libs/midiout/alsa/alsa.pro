include(../../../variables.pri)

TEMPLATE	= lib
LANGUAGE	= C++
TARGET		= midiout

INCLUDEPATH	+= ../common/ ../..
CONFIG          += plugin link_pkgconfig
PKGCONFIG	+= alsa

target.path	= $$OUTPUTPLUGINDIR
INSTALLS	+= target

FORMS		+= ../common/configuremididevice.ui \
		   ../common/configuremidiout.ui

HEADERS		+= ../common/configuremididevice.h \
		   ../common/configuremidiout.h \
		   mididevice-unix.h \
		   midiout-unix.h

SOURCES		+= ../common/configuremididevice.cpp \
		   ../common/configuremidiout.cpp \
		   mididevice-unix.cpp \
		   midiout-unix.cpp
