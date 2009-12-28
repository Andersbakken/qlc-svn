TEMPLATE = app
LANGUAGE = C++
TARGET = testewing

QT += network testlib
CONFIG += release warn_on
INCLUDEPATH += ../../common

SOURCES = testeplaybackwing.cpp \
	  testeprogramwing.cpp \
	  testeshortcutwing.cpp \
	  main.cpp \
	  \
	  ../ewing.cpp \
	  ../eplaybackwing.cpp \
	  ../eprogramwing.cpp \
	  ../eshortcutwing.cpp

HEADERS = testeplaybackwing.h \
	  testeprogramwing.h \
	  testeshortcutwing.h \
	  \
	  ../ewing.h \
	  ../eplaybackwing.h \
	  ../eprogramwing.h \
	  ../eshortcutwing.h
