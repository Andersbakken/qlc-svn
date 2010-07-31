include(../../../variables.pri)

TEMPLATE = app
LANGUAGE = C++
TARGET   = test_ewing

QT 	+= network testlib
CONFIG 	-= app_bundle
INCLUDEPATH += ../../common
QTPLUGIN =

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
