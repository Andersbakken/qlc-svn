TEMPLATE = app
LANGUAGE = C++
TARGET = testewing

QT += network testlib
CONFIG += release warn_on
INCLUDEPATH += ../../

SOURCES = testeplaybackwing.cpp \
	  ../ewing.cpp \
	  ../eplaybackwing.cpp

HEADERS = testeplaybackwing.h \
	  ../ewing.h \
	  ../eplaybackwing.h
