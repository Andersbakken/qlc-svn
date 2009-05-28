TEMPLATE = app
LANGUAGE = C++
CONFIG	+= warn_on
QT 	+= testlib xml
DEFINES += QLC_UNIT_TEST

TARGET 	= test

INCLUDEPATH += ../ ../../

HEADERS += qlcphysical_test.h \
	   ../qlcphysical.h \
	   qlcfixturemode_test.h \
	   ../qlcfixturemode.h \
	   qlcchannel_test.h \
	   ../qlcchannel.h \
	   qlccapability_test.h \
	   ../qlccapability.h \
	   qlcfixturedef_test.h \
	   ../qlcfixturedef.h \
	   \
	   qlcinputchannel_test.h \
	   ../qlcinputchannel.h \
	   \
	   ../qlcfile.h

SOURCES += qlcphysical_test.cpp \
	   ../qlcphysical.cpp \
	   qlcfixturemode_test.cpp \
	   ../qlcfixturemode.cpp \
	   qlcchannel_test.cpp \
	   ../qlcchannel.cpp \
	   qlccapability_test.cpp \
	   ../qlccapability.cpp \
	   qlcfixturedef_test.cpp \
	   ../qlcfixturedef.cpp \
	   \
	   qlcinputchannel_test.cpp \
	   ../qlcinputchannel.cpp \
	   \
	   ../qlcfile.cpp

SOURCES += main.cpp
