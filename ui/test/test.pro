include(../../variables.pri)

TEMPLATE = app
LANGUAGE = C++
TARGET   = test_ui

QMAKE_CXXFLAGS	-= -Werror
CONFIG		+= qtestlib
QT 		+= xml
QT		+= gui
QTPLUGIN	 =

INCLUDEPATH += ../../plugins/interfaces
INCLUDEPATH += ../src
DEPENDPATH  += ../src

HEADERS = vcwidgetproperties_test.h \
	  ../src/vcwidgetproperties.h
SOURCES = vcwidgetproperties_test.cpp \
	  ../src/vcwidgetproperties.cpp \
	  main.cpp
