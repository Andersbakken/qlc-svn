include(../../variables.pri)

TEMPLATE	= lib
LANGUAGE	= C++
TARGET		= midiout

INCLUDEPATH	+= . ../../libs/
CONFIG          += plugin warn_on release
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

###############################################################################
# UNIX sources
###############################################################################

unix:FORMS	+= configuremididevice.ui \
		   configuremidiout.ui

unix:HEADERS	+= configuremididevice.h \
		   configuremidiout.h \
		   mididevice.h \
		   midiout.h

unix:SOURCES	+= configuremididevice.cpp \
		   configuremidiout.cpp \
		   mididevice.cpp \
		   midiout.cpp

###############################################################################
# Win32 sources
###############################################################################

win32:FORMS	+= configuremididevice.ui \
		   configuremidiout.ui

#win32:SOURCES += configuremidiout.cpp \
#		 mididevice-win32.cpp \#		 midiout-win32.cpp

#win32:HEADERS += configuremidiout.h \
#		  mididevice-win32.h \
#		  midiout-win32.h

