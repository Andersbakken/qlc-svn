include(../../../variables.pri)

TEMPLATE = app
LANGUAGE = C++
TARGET   = test_ewing

QT 	+= network testlib
CONFIG 	-= app_bundle
INCLUDEPATH += ../../common
QTPLUGIN =

TEST_SOURCES = testeplaybackwing.cpp testeprogramwing.cpp testeshortcutwing.cpp main.cpp
PLUGIN_SOURCES = ../ewing.cpp ../eplaybackwing.cpp ../eprogramwing.cpp ../eshortcutwing.cpp
SOURCES = $$TEST_SOURCES $$PLUGIN_SOURCES

TEST_HEADERS = testeplaybackwing.h testeprogramwing.h testeshortcutwing.h
PLUGIN_HEADERS = ../ewing.h ../eplaybackwing.h ../eprogramwing.h ../eshortcutwing.h
HEADERS = $$TEST_HEADERS $$PLUGIN_HEADERS

GCOV_FILES = $$PLUGIN_SOURCES $$PLUGIN_HEADERS
include(../../../coverage.pri)
