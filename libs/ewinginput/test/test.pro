include(../../../variables.pri)

TEMPLATE = app
LANGUAGE = C++
TARGET   = test_ewing

QT 	+= core gui network testlib
CONFIG 	-= app_bundle
QTPLUGIN =

INCLUDEPATH += ../../common
INCLUDEPATH += ../src
unix:LIBS += -L../src -lewinginput

SOURCES += testeplaybackwing.cpp \
	   testeprogramwing.cpp \
	   testeshortcutwing.cpp \
	   main.cpp
HEADERS += testeplaybackwing.h \
	   testeprogramwing.h \
	   testeshortcutwing.h

# Hmm... Windows can't link to the plugin DLL
win32:SOURCES += ../src/eplaybackwing.cpp \
		 ../src/eshortcutwing.cpp \
		 ../src/eprogramwing.cpp \
		 ../src/ewing.cpp
win32:HEADERS += ../src/eplaybackwing.h \
		 ../src/eshortcutwing.h \
		 ../src/eprogramwing.h \
		 ../src/ewing.h
