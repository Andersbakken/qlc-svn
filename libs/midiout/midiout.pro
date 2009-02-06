include(../../variables.pri)

TEMPLATE	= lib
LANGUAGE	= C++
TARGET		= midiout

INCLUDEPATH	+= . ../../libs/
CONFIG          += plugin
unix:CONFIG	+= link_pkgconfig

###############################################################################
# Installation
###############################################################################

target.path	= $$OUTPUTPLUGINDIR
!macx:INSTALLS	+= target

macx:DESTDIR    = ../../main/qlc.app/Contents/Plugins/output

###############################################################################
# Linking
###############################################################################

win32:LIBS	+= -lwinmm
unix:PKGCONFIG	+= alsa
unix:LIBS	+= -lasound

###############################################################################
# UNIX sources
###############################################################################

unix:FORMS	+= configuremididevice.ui \
		   configuremidiout.ui

unix:HEADERS	+= configuremididevice.h \
		   configuremidiout.h \
		   mididevice-unix.h \
		   midiout-unix.h

unix:SOURCES	+= configuremididevice.cpp \
		   configuremidiout.cpp \
		   mididevice-unix.cpp \
		   midiout-unix.cpp

###############################################################################
# Win32 sources
###############################################################################

win32:FORMS	+= configuremididevice.ui \
		   configuremidiout.ui

win32:SOURCES	+= configuremididevice.cpp \
		   configuremidiout.cpp \
		   mididevice-win32.cpp \
		   midiout-win32.cpp

win32:HEADERS	+= configuremididevice.h \
		   configuremidiout.h \
		   mididevice-win32.h \
		   midiout-win32.h \
		   midiprotocol.h
