TEMPLATE = app
LANGUAGE = C++
CONFIG	+= warn_on release
QT 	+= testlib xml
DEFINES += QLC_UNIT_TEST

TARGET 	= test

INCLUDEPATH += ../

HEADERS += qlcphysical_test.h
HEADERS += ../qlcphysical.h

SOURCES += main.cpp
SOURCES += qlcphysical_test.cpp
SOURCES += ../qlcphysical.cpp