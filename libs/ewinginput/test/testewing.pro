TEMPLATE = app
LANGUAGE = C++
TARGET = testewing

QT += network testlib
CONFIG += release warn_on
INCLUDEPATH += ../../

SOURCES = testeplaybackwing.cpp \
	  testeshortcutwing.cpp \
	  main.cpp \
	  \
	  ../ewing.cpp \
	  ../eplaybackwing.cpp \
	  ../eshortcutwing.cpp

HEADERS = testeplaybackwing.h \
	  testeshortcutwing.h \
	  \
	  ../ewing.h \
	  ../eplaybackwing.h \
	  ../eshortcutwing.h
