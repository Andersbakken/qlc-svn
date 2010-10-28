include(../../../variables.pri)

TEMPLATE = app
LANGUAGE = C++
TARGET   = test_ewing

QT     += core gui network testlib

INCLUDEPATH += ../../interfaces
INCLUDEPATH += ../src
LIBS   += -L../src -lewinginput

SOURCES += testeplaybackwing.cpp \
           testeprogramwing.cpp \
           testeshortcutwing.cpp \
           main.cpp

HEADERS += testeplaybackwing.h \
           testeprogramwing.h \
           testeshortcutwing.h
