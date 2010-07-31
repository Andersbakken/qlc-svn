include(../../variables.pri)

TEMPLATE = app
LANGUAGE = C++
TARGET   = test_ui

QMAKE_CXXFLAGS	-= -Werror
CONFIG		+= qtestlib
macx:CONFIG	-= app_bundle
QT 		+= xml
QT		+= gui
QTPLUGIN	 =

INCLUDEPATH += ../src
INCLUDEPATH += ../../libs/common
DEPENDPATH  += ../src

unix:LIBS   += ../../libs/common/libqlccommon.a
win32:{
	CONFIG(release, debug|release) LIBS += ../../libs/common/release/libqlccommon.a
	CONFIG(debug, debug|release) LIBS += ../../libs/common/debug/libqlccommon.a
}

HEADERS = vcwidgetproperties_test.h \
	  ../src/vcwidgetproperties.h
SOURCES = vcwidgetproperties_test.cpp \
	  ../src/vcwidgetproperties.cpp \
	  main.cpp
