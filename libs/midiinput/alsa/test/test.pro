TEMPLATE = app
TARGET = test
QT += testlib

INCLUDEPATH += ../ ../../ ../../common ../../../

CONFIG	+= link_pkgconfig
PKGCONFIG += alsa

HEADERS += unix-mididevice_test.h \
	   unix-midiinput_test.h \
	   midiinputevent_test.h \
	   ../../common/midiinputevent.h \
	   ../../common/configuremidiinput.h \
	   ../unix-mididevice.h \
	   ../unix-midiinput.h \
	   ../unix-midipoller.h

SOURCES += main.cpp \
	   midiinputevent_test.cpp \
	   unix-mididevice_test.cpp \
	   unix-midiinput_test.cpp \
	   ../../common/midiinputevent.cpp \
	   ../../common/configuremidiinput.cpp \
	   ../unix-mididevice.cpp \
	   ../unix-midiinput.cpp \
	   ../unix-midipoller.cpp
