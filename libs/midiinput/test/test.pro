TEMPLATE = app
TARGET = test
QT += testlib

INCLUDEPATH += ../ ../../

HEADERS += midiinputevent_test.h \
	   ../midiinputevent.h \
	   ../configuremidiinput.h

SOURCES += main.cpp \
	   midiinputevent_test.cpp \
	   ../midiinputevent.cpp \
	   ../configuremidiinput.cpp

unix:{
	CONFIG	+= link_pkgconfig
	PKGCONFIG += alsa

	HEADERS += unix-mididevice_test.h \
		   unix-midiinput_test.h \
		   ../unix-mididevice.h \
		   ../unix-midiinput.h \
		   ../unix-midipoller.h

	SOURCES += unix-mididevice_test.cpp \
		   unix-midiinput_test.cpp \
		   ../unix-mididevice.cpp \
		   ../unix-midiinput.cpp \
		   ../unix-midipoller.cpp
}
