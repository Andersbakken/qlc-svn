include(../../../variables.pri)

TEMPLATE = app
LANGUAGE = C++
TARGET   = test_common

CONFIG	-= app_bundle
QT 	+= testlib xml
QTPLUGIN =

unix:DEFINES += INTERNAL_FIXTUREDIR=\\\"../../../fixtures/\\\"
win32:DEFINES += INTERNAL_FIXTUREDIR=\\\"../../../../fixtures/\\\"

INCLUDEPATH += ..
DEPENDPATH  += ..

unix:LIBS	+= ../libqlccommon.a
win32:{
	CONFIG(release, debug|release) LIBS += ../release/libqlccommon.a
	CONFIG(debug, debug|release) LIBS += ../debug/libqlccommon.a
}

HEADERS += qlcphysical_test.h \
	   qlcfixturemode_test.h \
	   qlcchannel_test.h \
	   qlccapability_test.h \
	   qlcfixturedef_test.h \
	   qlcfixturedefcache_test.h \
	   qlcinputchannel_test.h \
	   qlcinputprofile_test.h \
	   qlcmacros_test.h

SOURCES += qlcphysical_test.cpp \
	   qlcfixturemode_test.cpp \
	   qlcchannel_test.cpp \
	   qlccapability_test.cpp \
	   qlcfixturedef_test.cpp \
	   qlcfixturedefcache_test.cpp \
	   qlcinputchannel_test.cpp \
	   qlcinputprofile_test.cpp \
	   qlcmacros_test.cpp

SOURCES += main.cpp
