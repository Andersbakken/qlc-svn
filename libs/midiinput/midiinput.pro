TEMPLATE 	= lib
LANGUAGE 	= C++
TARGET 		= midiinput

INCLUDEPATH	+= . ../../libs/
CONFIG          += plugin warn_on release

###############################################################################
# Installation
###############################################################################

unix:target.path	= /usr/lib/qlc/input
win32:target.path	= C:\QLC\plugins\Input
!macx:INSTALLS	+= target

macx:DESTDIR    = ../../main/qlc.app/Contents/Plugins/input

###############################################################################
# Linking
###############################################################################

win32:LIBS	+= -lwinmm

###############################################################################
# UNIX sources
###############################################################################

unix:FORMS	+= configuremidiinput.ui

unix:HEADERS	+= configuremidiinput.h \
		   midiinputevent.h \
		   unix-mididevice.h \
		   unix-midiinput.h \
		   unix-midipoller.h

unix:SOURCES 	+= configuremidiinput.cpp \
		   midiinputevent.cpp \
		   unix-mididevice.cpp \
		   unix-midiinput.cpp \
		   unix-midipoller.cpp

###############################################################################
# WIN32 sources
###############################################################################

win32:FORMS	+= configuremidiinput.ui

win32:HEADERS	+= configuremidiinput.h \
   		   midiinputevent.h \
		   win32-mididevice.h \
		   win32-midiinput.h

win32:SOURCES	+= configuremidiinput.cpp \
		   midiinputevent.cpp \
		   win32-mididevice.cpp \
		   win32-midiinput.cpp
