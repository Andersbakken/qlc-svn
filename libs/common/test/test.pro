TEMPLATE = app
LANGUAGE = C++
CONFIG	+= warn_on release
QT 	+= testlib xml
DEFINES += QLC_UNIT_TEST

TARGET 	= test

INCLUDEPATH += ../

HEADERS += qlcphysical_test.h \
	   ../qlcphysical.h \
	   qlcfixturemode_test.h \
	   ../qlcfixturemode.h \
	   \
	   ../qlcchannel.h \
	   ../qlccapability.h \
	   ../qlcfixturedef.h \
	   ../qlcfile.h

SOURCES += qlcphysical_test.cpp \
	   ../qlcphysical.cpp \
	   qlcfixturemode_test.cpp \
	   ../qlcfixturemode.cpp \
	   \
	   ../qlcchannel.cpp \
	   ../qlccapability.cpp \
	   ../qlcfixturedef.cpp \
	   ../qlcfile.cpp

SOURCES += main.cpp
