TEMPLATE = app
TARGET = test
QT += testlib

INCLUDEPATH += ../ ../../

HEADERS += midiinputevent_test.h \
	   ../midiinputevent.h

SOURCES += main.cpp \
	   midiinputevent_test.cpp \
	   ../midiinputevent.cpp
