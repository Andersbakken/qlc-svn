include(../../../variables.pri)

TEMPLATE = app
LANGUAGE = C++
TARGET   = test_ewing

QT 	+= core gui network testlib
CONFIG 	-= app_bundle
QTPLUGIN =

INCLUDEPATH += ../../interfaces
INCLUDEPATH += ../src
unix:LIBS += -L../src -lewinginput

win32 {
	CONFIG(release, debug|release) LIBS += -L../src/release -lewinginput
	CONFIG(debug, debug|release) LIBS += -L../src/debug -lewinginput
}

SOURCES += testeplaybackwing.cpp testeprogramwing.cpp testeshortcutwing.cpp main.cpp
HEADERS += testeplaybackwing.h testeprogramwing.h testeshortcutwing.h
