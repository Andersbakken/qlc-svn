TEMPLATE 	= lib
LANGUAGE 	= C++
TARGET 		= midiinput

INCLUDEPATH	+= . ../../libs/
CONFIG          += plugin warn_on debug_and_release build_all

###############################################################################
# Installation
###############################################################################

unix:target.path	= /usr/lib/qlc/input
win32:target.path	= C:\QLC\plugins\Input
INSTALLS	+= target

###############################################################################
# Linking
###############################################################################

win32:LIBS	+= -lwinmm

###############################################################################
# UNIX sources
###############################################################################

unix:FORMS	+= configuremidiinput.ui

unix:HEADERS	+= configuremidiinput.h \
		   mididevice.h \
		   midiinput.h \
		   midiinputevent.h \
		   midipoller.h

unix:SOURCES 	+= configuremidiinput.cpp \
		   unix-mididevice.cpp \
		   unix-midipoller.cpp \
		   unix-midiinput.cpp \
		   midiinputevent.cpp

###############################################################################
# WIN32 sources
###############################################################################

win32:HEADERS	+= win32-mididevice.h \
		   win32-midiinput.h \
   		   midiinputevent.h


win32:SOURCES	+= win32-mididevice.cpp \
		   win32-midiinput.cpp \
		   midiinputevent.cpp


